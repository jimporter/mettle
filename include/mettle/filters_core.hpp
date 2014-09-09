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
    std::stringstream s;
    s << *begin;
    for(++begin; begin != end; ++begin)
      s << delim << *begin;
    return s.str();
  }
}

using filter_result = std::pair<test_action, std::string>;

struct default_filter {
  filter_result operator ()(const attributes &) const {
    return {test_action::indeterminate, ""};
  }
};

inline filter_result filter_by_attr(const attributes &attrs) {
  for(const auto &attr : attrs) {
    if(attr.attribute.action() == test_action::skip)
      return {test_action::skip, detail::join(attr.value, ", ")};
  }
  return {test_action::run, ""};
}

} // namespace mettle

#endif
