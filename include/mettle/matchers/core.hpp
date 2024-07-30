#ifndef INC_METTLE_MATCHERS_CORE_HPP
#define INC_METTLE_MATCHERS_CORE_HPP

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include "../output.hpp"
#include "../detail/any_capture.hpp"
#include "result.hpp"

namespace mettle {

  struct matcher_tag {};

  template<typename T>
  concept any_matcher = std::derived_from<
    std::remove_reference_t<T>, matcher_tag
  >;

  namespace detail {
    template<typename T>
    inline auto matcher_desc(T &&t) {
      if constexpr(any_matcher<T>) {
        return std::forward<T>(t).desc();
      } else {
        return to_printable(std::forward<T>(t));
      }
    }
  }

  template<typename T, typename F>
  class basic_matcher : public matcher_tag {
  public:
    basic_matcher(detail::any_capture<T> thing, F f, std::string prefix,
                  std::string suffix = "")
      : thing_(std::move(thing)), f_(std::move(f)),
        prefix_(std::move(prefix)), suffix_(std::move(suffix)) {}

    template<typename U>
    decltype(auto) operator ()(U &&actual) const {
      return f_(std::forward<U>(actual), thing_.value);
    }

    std::string desc() const {
      std::ostringstream ss;
      ss << prefix_ << detail::matcher_desc(thing_.value) << suffix_;
      return ss.str();
    }
  private:
    detail::any_capture<T> thing_;
    F f_;
    std::string prefix_, suffix_;
  };

  template<typename F>
  class basic_matcher<void, F> : public matcher_tag {
  public:
    basic_matcher(F f, std::string desc)
      : f_(std::move(f)), desc_(std::move(desc)) {}

    template<typename U>
    decltype(auto) operator ()(U &&actual) const {
      return f_(std::forward<U>(actual));
    }

    const std::string & desc() const {
      return desc_;
    }
  private:
    F f_;
    std::string desc_;
  };

  template<typename T, typename F>
  basic_matcher(T, F, std::string) -> basic_matcher<T, F>;
  template<typename T, typename F>
  basic_matcher(T, F, std::string, std::string) -> basic_matcher<T, F>;
  template<typename F>
  basic_matcher(F, std::string) -> basic_matcher<void, F>;

  template<typename T>
  auto equal_to(T &&expected) {
    return basic_matcher(std::forward<T>(expected), std::equal_to<>(), "");
  }

  inline auto anything() {
    return basic_matcher([](const auto &) -> bool {
      return true;
    }, "anything");
  }

  template<typename T>
  inline auto ensure_matcher(T &&t) {
    if constexpr(any_matcher<T>) {
      return std::forward<T>(t);
    } else {
      return equal_to(std::forward<T>(t));
    }
  }

  template<typename T>
  struct ensure_matcher_type : public std::remove_reference<
    decltype(ensure_matcher(std::declval<T>()))
  > {};

  template<typename T>
  using ensure_matcher_t = typename ensure_matcher_type<T>::type;

  template<typename T>
  inline auto is_not(T &&thing) {
    return basic_matcher(
      ensure_matcher(std::forward<T>(thing)),
      [](auto &&actual, auto &&matcher) {
        return !matcher(std::forward<decltype(actual)>(actual));
      }, "not "
    );
  }

  template<typename T>
  inline auto describe(T &&matcher, const std::string &desc) {
    return basic_matcher(std::forward<T>(matcher), desc);
  }

  template<typename Filter, typename Matcher>
  auto filter(Filter &&f, Matcher &&matcher, const std::string &desc = "") {
    return basic_matcher(
      std::forward<Matcher>(matcher),
      [f = std::forward<Filter>(f), desc](auto &&actual, auto &&matcher) {
        auto filtered = f(std::forward<decltype(actual)>(actual));
        auto result = matcher(filtered);
        std::ostringstream ss;
        ss << desc << matcher_message(result, filtered);
        return match_result(result, ss.str());
      }, desc
    );
  }

} // namespace mettle

#endif
