#ifndef INC_METTLE_MATCHERS_ARRAY_HPP
#define INC_METTLE_MATCHERS_ARRAY_HPP

#include <tuple>

#include "core.hpp"

namespace mettle {

template<typename T>
auto member(T &&thing) {
  auto matcher = ensure_matcher(std::forward<T>(thing));
  return make_matcher([matcher](auto &&value) -> bool {
    for(auto &i : value) {
      if(matcher(i))
        return true;
    }
    return false;
  }, "member " + matcher.desc());
}

template<typename T>
auto each(T &&thing) {
  auto matcher = ensure_matcher(std::forward<T>(thing));
  return make_matcher([matcher](auto &&value) -> bool {
    for(auto &i : value) {
      if(!matcher(i))
        return false;
    }
    return true;
  }, "each " + matcher.desc());
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
          if(i == end || !matcher(*i++)) {
            early_exit = true;
            return false;
          }
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

} // namespace mettle

#endif
