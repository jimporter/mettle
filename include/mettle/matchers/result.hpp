#ifndef INC_METTLE_MATCHERS_RESULT_HPP
#define INC_METTLE_MATCHERS_RESULT_HPP

#include <ostream>
#include <string>

namespace mettle {

struct match_result {
  match_result(bool matched, std::string message = "")
    : matched(matched), message(std::move(message)) {}
  match_result(const match_result &matched, const std::string &)
    : match_result(matched) {}
  match_result(match_result &&matched, const std::string &)
    : match_result(std::move(matched)) {}

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

namespace detail {
  template<typename T>
  class message_impl {
  public:
    message_impl(const match_result &result, const T &fallback)
      : result(result), fallback(fallback) {}

    friend std::ostream &
    operator <<(std::ostream &os, const message_impl<T> &m) {
      if(!m.result.message.empty())
        return os << m.result.message;
      else
        return os << m.fallback;
    }
  private:
    const match_result &result;
    const T &fallback;
  };
}

template<typename T>
inline detail::message_impl<T>
matcher_message(const match_result &result, const T &fallback) {
  return {result, fallback};
}

template<typename T>
inline const T &
matcher_message(bool, const T &fallback) {
  return fallback;
}

} // namespace mettle

#endif
