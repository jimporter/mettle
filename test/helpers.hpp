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

std::string to_printable(const attr_action &action) {
  switch(action) {
  case attr_action::run:
    return "attr_action::run";
  case attr_action::skip:
    return "attr_action::skip";
  case attr_action::hide:
    return "attr_action::hide";
  }
}

bool operator ==(const attr_instance &lhs, const attr_instance &rhs) {
  return lhs.attribute.name() == rhs.attribute.name() && lhs.value == rhs.value;
}

} // namespace mettle

#endif
