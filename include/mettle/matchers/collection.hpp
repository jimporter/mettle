#ifndef INC_METTLE_MATCHERS_COLLECTION_HPP
#define INC_METTLE_MATCHERS_COLLECTION_HPP

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <sstream>
#include <tuple>

#include "core.hpp"
#include "../detail/move_if.hpp"

namespace mettle {

template<typename T>
auto member(T &&thing) {
  return make_matcher(
    ensure_matcher(std::forward<T>(thing)),
    [](const auto &value, auto &&matcher) -> bool {
      for(const auto &i : value) {
        if(matcher(i))
          return true;
      }
      return false;
    }, "member "
  );
}

template<typename T>
auto each(T &&thing) {
  return make_matcher(
    ensure_matcher(std::forward<T>(thing)),
    [](const auto &value, auto &&matcher) -> bool {
      for(const auto &i : value) {
        if(!matcher(i))
          return false;
      }
      return true;
    }, "each "
  );
}

template<typename T, typename U>
auto each(T begin, T end, U &&meta_matcher) {
  using namespace detail;
  using Matcher = decltype(meta_matcher(*begin));
  std::vector<Matcher> matchers;
  for(; begin != end; ++begin)
    matchers.push_back(meta_matcher(*begin));

  std::string desc = "[" + stringify(joined(matchers, [](const auto &matcher) {
    return matcher.desc();
  })) + "]";

  return make_matcher(
    [matchers = std::move(matchers)](const auto &value) -> bool {
      auto i = std::begin(value), end = std::end(value);
      for(auto &&matcher : matchers) {
        if(i == end || !matcher(*i))
          return false;
        ++i;
      }
      return i == end;
    }, desc
  );
}

template<typename T, typename U>
inline auto each(T &thing, U &&meta_matcher) {
  return each(std::begin(thing), std::end(thing),
              std::forward<U>(meta_matcher));
}

template<typename T, typename U>
inline auto each(T &&thing, U &&meta_matcher) {
  return each(std::make_move_iterator(std::begin(thing)),
              std::make_move_iterator(std::end(thing)),
              std::forward<U>(meta_matcher));
}

template<typename T, typename U>
inline auto each(std::initializer_list<T> list, U &&meta_matcher) {
  return each(list.begin(), list.end(), std::forward<U>(meta_matcher));
}

namespace detail {
  template<typename ...T>
  class array_impl : public matcher_tag {
  public:
    array_impl(T ...matchers) : matchers_(std::move(matchers)...) {}

    template<typename U>
    bool operator ()(const U &value) const {
      bool good = true;
      auto i = std::begin(value), end = std::end(value);
      tuple_for_until(matchers_, [&i, &end, &good](const auto &m) {
        good = (i != end && m(*i));
        if(!good)
          return true;
        ++i;
        return false;
      });
      return good && i == end;
    }

    std::string desc() const {
      return "[" + stringify(tuple_joined(matchers_, [](auto &&matcher) {
        return matcher.desc();
      })) + "]";
    }
  private:
    std::tuple<T...> matchers_;
  };
}

template<typename ...T>
inline auto array(T &&...things) {
  return detail::array_impl<ensure_matcher_t<T>...>(
    ensure_matcher(std::forward<T>(things))...
  );
}

auto sorted() {
  return make_matcher([](const auto &value) {
    return std::is_sorted(std::begin(value), std::end(value));
  }, "sorted");
}

template<typename T>
auto sorted(T &&compare) {
  return make_matcher(
    std::forward<T>(compare),
    [](const auto &value, auto &&compare) {
    return std::is_sorted(std::begin(value), std::end(value), compare);
  }, "sorted by ");
}

} // namespace mettle

#endif
