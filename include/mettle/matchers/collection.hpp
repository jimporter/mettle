#ifndef INC_METTLE_MATCHERS_COLLECTION_HPP
#define INC_METTLE_MATCHERS_COLLECTION_HPP

#include <algorithm>
#include <tuple>

#include "core.hpp"
#include "tuple_algorithm.hpp"

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

namespace detail {
  template<typename ...T>
  class array_impl : public matcher_tag {
  public:
    using tuple_type = std::tuple<typename ensure_matcher_type<T>::type...>;

    array_impl(T &&...matchers)
      : matchers_(ensure_matcher(std::forward<T>(matchers))...) {}

    template<typename U>
    bool operator ()(const U &value) const {
      bool good = true;
      auto i = std::begin(value), end = std::end(value);
      detail::tuple_for_until(matchers_, [&i, &end, &good](const auto &m) {
        good = (i != end && m(*i));
        if(!good)
          return true;
        ++i;
        return false;
      });
      return good && i == end;
    }

    std::string desc() const {
      std::ostringstream ss;
      bool first = true;
      ss << "[";
      detail::tuple_for_each(matchers_, [&ss, &first](const auto &matcher) {
        if(!first)
          ss << ", ";
        ss << matcher.desc();
        first = false;
      });
      ss << "]";
      return ss.str();
    }
  private:
    tuple_type matchers_;
  };
}

template<typename ...T>
inline auto array(T &&...matchers) {
  return detail::array_impl<T...>(std::forward<T>(matchers)...);
}

auto sorted() {
  return make_matcher([](const auto &value) {
    return std::is_sorted(std::begin(value), std::end(value));
  }, "sorted");
}

template<typename T>
auto sorted(const T &comparator) {
  return make_matcher([comparator](const auto &value) {
    return std::is_sorted(std::begin(value), std::end(value), comparator);
  }, "sorted by " + type_name<T>());
}

} // namespace mettle

#endif
