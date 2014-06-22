#ifndef INC_METTLE_MATCHERS_COMBINATORIC_HPP
#define INC_METTLE_MATCHERS_COMBINATORIC_HPP

#include <functional>
#include <tuple>

#include "core.hpp"

namespace mettle {

namespace detail {
  template<typename ...T>
  class reduce_impl : public matcher_tag {
  public:
    using reducer_type = std::function<bool(bool, bool)>;
    using tuple_type = std::tuple<typename ensure_matcher_type<T>::type...>;

    reduce_impl(const std::string &desc, const reducer_type &reducer,
                bool initial, T &&...matchers)
      : desc_(desc), reducer_(reducer), initial_(initial),
        matchers_(ensure_matcher(std::forward<T>(matchers))...) {}

    template<typename U>
    bool operator ()(const U &value) const {
      return detail::reduce_tuple(
        matchers_, [&value, this](bool a, auto &&b, bool &early_exit) {
          bool result = reducer_(a, b(value));
          early_exit = (result != initial_);
          return result;
        }, initial_
      );
    }

    std::string desc() const {
      std::stringstream s;
      s << desc_ << "(";
      detail::reduce_tuple(matchers_, [&s](bool first, const auto &matcher,
                                           bool &) {
        if(!first)
          s << ", ";
        s << matcher.desc();
        return false;
      }, true);
      s << ")";
      return s.str();
    }
  private:
    std::string desc_;
    reducer_type reducer_;
    bool initial_;
    tuple_type matchers_;
  };
}

// We'd call these any_of and all_of like the std:: functions, but doing so
// would introduce an ambiguity where the std:: versions could be preferred via
// ADL if we pass in three arguments.

template<typename ...T>
inline auto any(T &&...matchers) {
  return detail::reduce_impl<T...>(
    "any of", std::logical_or<bool>(), false, std::forward<T>(matchers)...
  );
}

template<typename ...T>
inline auto all(T &&...matchers) {
  return detail::reduce_impl<T...>(
    "all of", std::logical_and<bool>(), true, std::forward<T>(matchers)...
  );
}

} // namespace mettle

#endif
