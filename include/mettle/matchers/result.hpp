#ifndef INC_METTLE_MATCHERS_RESULT_HPP
#define INC_METTLE_MATCHERS_RESULT_HPP

#include <ostream>
#include <string>

#include "../output/to_printable.hpp"

namespace mettle {

  struct match_result {
    match_result(bool matched, std::string message = "")
      : matched(matched), message(std::move(message)) {}

    bool matched;
    std::string message;

    operator bool() const {
      return matched;
    }
  };

  inline match_result operator !(const match_result &m) {
    return {!m.matched, m.message};
  }

  inline match_result operator !(match_result &&m) {
    return {!m.matched, std::move(m.message)};
  }

// Ignore warnings about const-qualified function types.
#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(push)
#  pragma warning(disable:4180)
#endif

  namespace detail {
    template<typename T>
    class message_impl {
    public:
      message_impl(const match_result &result, const T &fallback)
        : result(result), fallback(fallback) {}
      message_impl(const message_impl &) = delete;

      friend std::ostream &
      operator <<(std::ostream &os, const message_impl<T> &m) {
        if(!m.result.message.empty())
          return os << m.result.message;
        else
          return os << to_printable(m.fallback);
      }
    private:
      const match_result &result;
      const T &fallback;
    };
  }

#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(pop)
#endif

  template<typename T>
  inline const detail::message_impl<T>
  matcher_message(const match_result &result, const T &fallback) {
    return {result, fallback};
  }

  template<typename T>
  inline decltype(auto)
  matcher_message(bool, const T &fallback) {
    return to_printable(fallback);
  }

} // namespace mettle

#endif
