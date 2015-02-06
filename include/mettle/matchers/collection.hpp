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

namespace detail {

  template<typename Call, typename Result, bool Enable = true>
  struct enable_if_result : std::enable_if<
    std::is_same<typename std::result_of<Call>::type, Result>::value == Enable,
    typename std::result_of<Call>::type
  > {};

  template<typename Call, typename Result, bool Disable = true>
  using disable_if_result = enable_if_result<Call, Result, !Disable>;

  template<typename T>
  class member_impl : public matcher_tag {
  public:
    member_impl(std::string desc, bool initial, T matcher)
      : desc_(std::move(desc)), initial_(initial),
        matcher_(std::move(matcher)) {}

    template<typename U>
    auto operator ()(const U &value) const -> typename enable_if_result<
      T(decltype(*std::begin(value))), match_result
    >::type {
      std::ostringstream ss;
      bool good = initial_;
      ss << "[" << joined(value, [this, &good](auto &&i) {
        auto result = matcher_(i);
        if(result != initial_)
          good = result;
        return result.message;
      }) << "]";
      return {good, ss.str()};
    }

    template<typename U>
    auto operator ()(const U &value) const -> typename disable_if_result<
      T(decltype(*std::begin(value))), match_result
    >::type {
      for(const auto &i : value) {
        auto result = matcher_(i);
        if(result != initial_)
          return result;
      }
      return initial_;
    }

    std::string desc() const {
      return desc_ + matcher_.desc();
    }
  private:
    const std::string desc_;
    const bool initial_;
    T matcher_;
  };
}

template<typename T>
auto member(T &&thing) {
  return detail::member_impl<ensure_matcher_t<T>>(
    "member ", false, ensure_matcher(std::forward<T>(thing))
  );
}

template<typename T>
auto each(T &&thing) {
  return detail::member_impl<ensure_matcher_t<T>>(
    "each ", true, ensure_matcher(std::forward<T>(thing))
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
