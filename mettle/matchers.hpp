#ifndef INC_METTLE_MATCHERS_HPP
#define INC_METTLE_MATCHERS_HPP

#include <sstream>
#include <string>
#include <utility>

#include "utils.hpp"

namespace mettle {

template<typename T, typename Matcher>
void expect(T &&value, Matcher &&matcher) {
  if (!matcher(value))
    throw expectation_error(matcher.desc(value));
}

template<typename T>
class equals_impl {
public:
  equals_impl(const T &value) : value_(value) {}

  template<typename U>
  bool operator() (U &&other) const {
    return other == value_;
  }

  template<typename U>
  std::string desc(U &&other) const {
    std::stringstream s;
    s << other << " == " << value_;
    return s.str();
  }
private:
  T value_;
};

template<typename T>
equals_impl<T> equals(const T &value) {
  return equals_impl<T>(value);
}

template<typename T>
class not_impl {
public:
  not_impl(const T &matcher) : matcher_(matcher) {}

  template<typename U>
  bool operator() (U &&value) const {
    return !matcher_(value);
  }

  template<typename U>
  std::string desc(U &&value) const {
    std::stringstream s;
    s << "not(" << matcher_.desc(value) << ")";
    return s.str();
  }
private:
  T matcher_;
};

template<typename T>
not_impl<T> is_not(const T &value) {
  return not_impl<T>(value);
}

} // namespace mettle

#endif
