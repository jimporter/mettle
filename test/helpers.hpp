#ifndef INC_METTLE_TEST_HELPERS_HPP
#define INC_METTLE_TEST_HELPERS_HPP

#include <mettle/suite/attributes.hpp>
#include <mettle/filters_core.hpp>

namespace mettle {

std::string to_printable(const attr_instance &attr) {
  std::ostringstream ss;
  ss << attr.attribute.name();
  if(!attr.value.empty()) {
    auto i = attr.value.begin();
    ss << "(" << to_printable(*i);
    for(++i; i != attr.value.end(); ++i)
      ss << ", " << to_printable(*i);
    ss << ")";
  }
  return ss.str();
}

std::string to_printable(const attributes &attrs) {
  std::ostringstream ss;
  ss << "{";
  if(!attrs.empty()) {
    auto i = attrs.begin();
    ss << to_printable(*i);
    for(++i; i != attrs.end(); ++i)
      ss << ", " << to_printable(*i);
  }
  ss << "}";
  return ss.str();
}

std::string to_printable(const test_action &action) {
  switch(action) {
  case test_action::run:
    return "test_action::run";
  case test_action::skip:
    return "test_action::skip";
  case test_action::hide:
    return "test_action::hide";
  case test_action::indeterminate:
    return "attr_action::indeterminate";
  }
}

std::string to_printable(const filter_result &result) {
  std::ostringstream ss;
  ss << "filter_result(" << to_printable(result.action) << ", "
     << to_printable(result.message) << ")";
  return ss.str();
}

bool operator ==(const attr_instance &lhs, const attr_instance &rhs) {
  return lhs.attribute.name() == rhs.attribute.name() && lhs.value == rhs.value;
}

bool operator ==(const filter_result &lhs, const filter_result &rhs) {
  return lhs.action == rhs.action && lhs.message == rhs.message;
}

} // namespace mettle

#endif
