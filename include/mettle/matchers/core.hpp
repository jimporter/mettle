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

  class desc_wrapper {
  public:
    desc_wrapper(std::string prefix, std::string suffix = "")
      : prefix_(std::move(prefix)), suffix_(std::move(suffix)) {}

    template<typename T>
    std::string format_desc(T &&t) const {
      std::ostringstream ss;
      ss << prefix_ << std::forward<T>(t) << suffix_;
      return ss.str();
    }
  private:
    std::string prefix_, suffix_;
  };

  template<typename T, typename Func>
  class basic_matcher : public matcher_tag {
  public:
    basic_matcher(detail::any_capture<T> thing, Func f, std::string prefix,
                  std::string suffix = "")
      : thing_(std::move(thing)), f_(std::move(f)),
        desc_(std::move(prefix), std::move(suffix)) {}

    template<typename U>
    decltype(auto) operator ()(U &&actual) const {
      return f_(std::forward<U>(actual), detail::unwrap_capture(thing_));
    }

    std::string desc() const {
      return desc_.format_desc(to_printable(detail::unwrap_capture(thing_)));
    }
  private:
    detail::any_capture<T> thing_;
    Func f_;
    desc_wrapper desc_;
  };

  template<typename Func>
  class basic_matcher<void, Func> : public matcher_tag {
  public:
    basic_matcher(Func f, std::string desc)
      : f_(std::move(f)), desc_(std::move(desc)) {}

    template<typename T>
    decltype(auto) operator ()(T &&actual) const {
      return f_(std::forward<T>(actual));
    }

    const std::string & desc() const {
      return desc_;
    }
  private:
    Func f_;
    std::string desc_;
  };

  template<typename T, typename Func>
  basic_matcher(T, Func, std::string) -> basic_matcher<T, Func>;
  template<typename T, typename Func>
  basic_matcher(T, Func, std::string, std::string) -> basic_matcher<T, Func>;
  template<typename Func>
  basic_matcher(Func, std::string) -> basic_matcher<void, Func>;

  template<any_matcher Matcher, typename Func>
  class transform_matcher : public matcher_tag {
  public:
    transform_matcher(Matcher matcher, Func f, std::string prefix,
                      std::string suffix = "")
      : matcher_(std::move(matcher)), f_(std::move(f)),
        desc_(std::move(prefix), std::move(suffix)) {}

    template<typename T>
    decltype(auto) operator ()(T &&actual) const {
      return f_(matcher_(std::forward<T>(actual)));
    }

    std::string desc() const {
      return desc_.format_desc(matcher_.desc());
    }
  private:
    Matcher matcher_;
    Func f_;
    desc_wrapper desc_;
  };

  template<typename Func, any_matcher Matcher>
  class filter_matcher : public matcher_tag {
  public:
    filter_matcher(Func f, Matcher matcher, std::string prefix,
                   std::string suffix = "")
      : f_(std::move(f)), matcher_(std::move(matcher)),
        desc_(std::move(prefix), std::move(suffix)) {}

    template<typename T>
    match_result operator ()(T &&actual) const {
      auto filtered = f_(std::forward<T>(actual));
      auto result = matcher_(filtered);
      return {result, desc_.format_desc(matcher_message(result, filtered))};
    }

    std::string desc() const {
      return desc_.format_desc(matcher_.desc());
    }
  private:
    Func f_;
    Matcher matcher_;
    desc_wrapper desc_;
  };

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
  inline decltype(auto) ensure_matcher(T &&t) {
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
    return transform_matcher(
      ensure_matcher(std::forward<T>(thing)),
      std::logical_not<>{}, "not "
    );
  }

  template<any_matcher Matcher>
  inline auto describe(Matcher &&matcher, const std::string &desc) {
    return basic_matcher(std::forward<Matcher>(matcher), desc);
  }

  template<typename Func, any_matcher Matcher>
  auto filter(Func &&f, Matcher &&matcher, const std::string &desc = "") {
    return filter_matcher(
      std::forward<Func>(f), std::forward<Matcher>(matcher), desc
    );
  }

} // namespace mettle

#endif
