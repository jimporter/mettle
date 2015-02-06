#ifndef INC_METTLE_MATCHERS_COMBINATORIC_HPP
#define INC_METTLE_MATCHERS_COMBINATORIC_HPP

#include <functional>
#include <tuple>

#include "core.hpp"
#include "../detail/tuple_algorithm.hpp"

namespace mettle {

namespace detail {
  template<typename Reducer, typename ...T>
  class reduce_impl : public matcher_tag {
  public:
    reduce_impl(std::string desc, bool initial, Reducer reducer, T ...matchers)
      : desc_(std::move(desc)), initial_(initial), reducer_(std::move(reducer)),
        matchers_(std::move(matchers)...) {}

    template<typename U>
    match_result operator ()(const U &value) const {
      match_result result = initial_;
      tuple_for_until(matchers_, [&, this](const auto &matcher) {
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
      ss << desc_ << "(" << tuple_joined(matchers_, [](auto &&matcher) {
        return matcher.desc();
      }) << ")";
      return ss.str();
    }
  private:
    const std::string desc_;
    const bool initial_;
    Reducer reducer_;
    std::tuple<T...> matchers_;
  };
}

// We'd call these any_of, all_of, and none_of like the std:: functions, but
// doing so would introduce an ambiguity where the std:: versions could be
// preferred via ADL if we pass in three arguments.

template<typename ...T>
inline auto any(T &&...things) {
  return detail::reduce_impl<std::logical_or<bool>, ensure_matcher_t<T>...>(
    "any of", false, std::logical_or<bool>(),
    ensure_matcher(std::forward<T>(things))...
  );
}

template<typename ...T>
inline auto all(T &&...things) {
  return detail::reduce_impl<std::logical_and<bool>, ensure_matcher_t<T>...>(
    "all of", true, std::logical_and<bool>(),
    ensure_matcher(std::forward<T>(things))...
  );
}

template<typename ...T>
inline auto none(T &&...things) {
  auto reducer = [](bool a, bool b) { return a && !b; };
  return detail::reduce_impl<decltype(reducer), ensure_matcher_t<T>...>(
    "none of", true, std::move(reducer),
    ensure_matcher(std::forward<T>(things))...
  );
}

} // namespace mettle

#endif
