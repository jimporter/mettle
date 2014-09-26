#ifndef INC_METTLE_FILTERS_CORE_HPP
#define INC_METTLE_FILTERS_CORE_HPP

#include "attributes.hpp"

namespace mettle {

namespace detail {
  template<typename T>
  std::string join(const T &t, const std::string &delim) {
    auto begin = t.begin(), end = t.end();
    if(begin == end)
      return "";
    std::ostringstream ss;
    ss << *begin;
    for(++begin; begin != end; ++begin)
      ss << delim << *begin;
    return ss.str();
  }
}

struct filter_result {
  filter_result() = default;
  filter_result(test_action action, std::string message = "")
    : action(action), message(std::move(message)) {}

  test_action action;
  std::string message;
};

struct default_filter {
  filter_result operator ()(const test_name &, const attributes &) const {
    return test_action::indeterminate;
  }
};

inline filter_result filter_by_attr(const attributes &attrs) {
  for(const auto &attr : attrs) {
    if(attr.attribute.action() == test_action::skip)
      return {test_action::skip, detail::join(attr.value, ", ")};
  }
  return test_action::run;
}

} // namespace mettle

#endif
