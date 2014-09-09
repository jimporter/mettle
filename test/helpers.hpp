#ifndef INC_METTLE_TEST_HELPERS_HPP
#define INC_METTLE_TEST_HELPERS_HPP

#include <mettle/attributes.hpp>

namespace mettle {

std::string to_printable(const attr_instance &attr) {
  std::stringstream s;
  s << attr.attribute.name();
  if(!attr.value.empty()) {
    auto i = attr.value.begin();
    s << "(" << to_printable(*i);
    for(++i; i != attr.value.end(); ++i)
      s << ", " << to_printable(*i);
    s << ")";
  }
  return s.str();
}

std::string to_printable(const attributes &attrs) {
  std::stringstream s;
  s << "{";
  if(!attrs.empty()) {
    auto i = attrs.begin();
    s << to_printable(*i);
    for(++i; i != attrs.end(); ++i)
      s << ", " << to_printable(*i);
  }
  s << "}";
  return s.str();
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

bool operator ==(const attr_instance &lhs, const attr_instance &rhs) {
  return lhs.attribute.name() == rhs.attribute.name() && lhs.value == rhs.value;
}

} // namespace mettle

#endif
