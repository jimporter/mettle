#ifndef INC_METTLE_MATCHERS_ARRAY_HPP
#define INC_METTLE_MATCHERS_ARRAY_HPP

#include <algorithm>
#include <tuple>

#include "core.hpp"

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
      auto i = std::begin(value), end = std::end(value);
      bool good = detail::reduce_tuple(
        matchers_, [&i, &end](bool, const auto &matcher, bool &early_exit) {
          if(i == end || !matcher(*i)) {
            early_exit = true;
            return false;
          }
          ++i;
          return true;
        }, true
      );
      return good && i == end;
    }

    std::string desc() const {
      std::stringstream s;
      s << "[";
      detail::reduce_tuple(matchers_, [&s](bool first, const auto &matcher,
                                           bool &) {
        if(!first)
          s << ", ";
        s << matcher.desc();
        return false;
      }, true);
      s << "]";
      return s.str();
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
