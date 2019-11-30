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
  struct is_matcher : public std::is_base_of<
    matcher_tag, typename std::remove_reference<T>::type
  > {};

  template<typename T>
  constexpr bool is_matcher_v = is_matcher<T>::value;

  namespace detail {
    template<typename T>
    inline auto matcher_desc(T &&matcher) -> std::enable_if_t<
      is_matcher_v<T>, decltype( std::forward<T>(matcher).desc() )
    > {
      return std::forward<T>(matcher).desc();
    }

    template<typename T>
    inline auto matcher_desc(T &&expected) -> std::enable_if_t<
      !is_matcher_v<T>, decltype( to_printable(std::forward<T>(expected)) )
    > {
      return to_printable(std::forward<T>(expected));
    }
  }

  template<typename T, typename F>
  class basic_matcher : public matcher_tag {
  public:
    basic_matcher(detail::any_capture<T> thing, F f, std::string prefix)
      : thing_(std::move(thing)), f_(std::move(f)),
        prefix_(std::move(prefix)) {}
    basic_matcher(detail::any_capture<T> thing, F f,
                  std::pair<std::string, std::string> format)
      : thing_(std::move(thing)), f_(std::move(f)),
        prefix_(std::move(format.first)), suffix_(std::move(format.second)) {}

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
    template<typename F2>
    basic_matcher(F2 &&f, std::string desc)
      : f_(std::forward<F2>(f)), desc_(std::move(desc)) {}

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

  template<typename T, typename F, typename String>
  inline auto make_matcher(T &&thing, F &&f, String &&prefix) {
    return basic_matcher<
      std::remove_reference_t<T>, std::remove_reference_t<F>
    >(std::forward<T>(thing), std::forward<F>(f), std::forward<String>(prefix));
  }

  template<typename T, typename F>
  inline auto
  make_matcher(T &&thing, F &&f, std::pair<std::string, std::string> format) {
    return basic_matcher<
      std::remove_reference_t<T>, std::remove_reference_t<F>
    >(std::forward<T>(thing), std::forward<F>(f), std::move(format));
  }

  template<typename F, typename String>
  inline auto make_matcher(F &&f, String &&desc) {
    return basic_matcher<
      void, std::remove_reference_t<F>
    >(std::forward<F>(f), std::forward<String>(desc));
  }

  template<typename T>
  auto equal_to(T &&expected) {
    return make_matcher(std::forward<T>(expected), std::equal_to<>(), "");
  }

  inline auto anything() {
    return make_matcher([](const auto &) -> bool {
      return true;
    }, "anything");
  }

  template<typename T>
  inline auto ensure_matcher(T &&matcher) -> std::enable_if_t<
    is_matcher_v<T>, decltype( std::forward<T>(matcher) )
  > {
    return std::forward<T>(matcher);
  }

  template<typename T>
  inline auto ensure_matcher(T &&expected) -> std::enable_if_t<
    !is_matcher_v<T>, decltype( equal_to(std::forward<T>(expected)) )
  > {
    return equal_to(std::forward<T>(expected));
  }

  template<typename T>
  struct ensure_matcher_type : public std::remove_reference<
    decltype(ensure_matcher(std::declval<T>()))
  > {};

  template<typename T>
  using ensure_matcher_t = typename ensure_matcher_type<T>::type;

  template<typename T>
  inline auto is_not(T &&thing) {
    return make_matcher(
      ensure_matcher(std::forward<T>(thing)),
      [](const auto &value, auto &&matcher) {
        return !matcher(value);
      }, "not "
    );
  }

  template<typename T>
  inline auto describe(T &&matcher, const std::string &desc) {
    return make_matcher(std::forward<T>(matcher), desc);
  }

  template<typename Filter, typename Matcher>
  auto filter(Filter &&f, Matcher &&matcher, const std::string &desc = "") {
    return make_matcher(
      std::forward<Matcher>(matcher),
      [f = std::forward<Filter>(f), desc](const auto &actual, auto &&matcher) {
        auto filtered = f(actual);
        auto result = matcher(filtered);
        std::ostringstream ss;
        ss << desc << matcher_message(result, filtered);
        return match_result(result, ss.str());
      }, desc
    );
  }

} // namespace mettle

#endif
