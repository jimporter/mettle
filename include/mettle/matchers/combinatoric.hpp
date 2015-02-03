#ifndef INC_METTLE_MATCHERS_COMBINATORIC_HPP
#define INC_METTLE_MATCHERS_COMBINATORIC_HPP

#include <functional>
#include <tuple>

#include "core.hpp"
#include "../detail/tuple_algorithm.hpp"

namespace mettle {

namespace detail {
  template<typename ...T>
  class reduce_impl : public matcher_tag {
  public:
    using reducer_type = std::function<bool(bool, bool)>;
    using tuple_type = std::tuple<typename ensure_matcher_type<T>::type...>;

    reduce_impl(std::string desc, reducer_type reducer, bool initial,
                T &&...matchers)
      : desc_(std::move(desc)), reducer_(std::move(reducer)), initial_(initial),
        matchers_(ensure_matcher(std::forward<T>(matchers))...) {}

    template<typename U>
    match_result operator ()(const U &value) const {
      match_result result = initial_;
      detail::tuple_for_until(matchers_, [&, this](const auto &matcher) {
        auto m = matcher(value);
        bool done = reducer_(initial_, m) != initial_;
        if(done)
          result = std::move(m);
        return done;
      });
      return result;
    }

    std::string desc() const {
      std::ostringstream ss;
      ss << desc_ << "(" << detail::tuple_joined(matchers_, [](auto &&matcher) {
        return matcher.desc();
      }) << ")";
      return ss.str();
    }
  private:
    std::string desc_;
    reducer_type reducer_;
    bool initial_;
    tuple_type matchers_;
  };
}

// We'd call these any_of, all_of, and none_of like the std:: functions, but
// doing so would introduce an ambiguity where the std:: versions could be
// preferred via ADL if we pass in three arguments.

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

template<typename ...T>
inline auto none(T &&...matchers) {
  return detail::reduce_impl<T...>(
    "none of", [](bool a, bool b) { return a && !b; }, true,
    std::forward<T>(matchers)...
  );
}

} // namespace mettle

#endif
