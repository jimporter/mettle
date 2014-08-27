#ifndef INC_METTLE_TEST_HELPERS_HPP
#define INC_METTLE_TEST_HELPERS_HPP

#include <mettle/attributes.hpp>

namespace mettle {

std::string ensure_printable(const attr_instance &attr) {
  std::stringstream s;
  s << attr.name();
  if(!attr.empty()) {
    auto i = attr.value().begin();
    s << "(" << ensure_printable(*i);
    for(++i; i != attr.value().end(); ++i)
      s << ", " << ensure_printable(*i);
    s << ")";
  }
  return s.str();
}

std::string ensure_printable(const attr_list &attrs) {
  std::stringstream s;
  s << "{";
  if(!attrs.empty()) {
    auto i = attrs.begin();
    s << ensure_printable(*i);
    for(++i; i != attrs.end(); ++i)
      s << ", " << ensure_printable(*i);
  }
  s << "}";
  return s.str();
}

std::string ensure_printable(const attr_action &action) {
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
  return lhs.name() == rhs.name() && std::equal(
    lhs.value().begin(), lhs.value().end(),
    rhs.value().begin(), rhs.value().end()
  );
}

bool operator ==(const attr_list &lhs, const attr_list &rhs) {
  return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

} // namespace mettle

#endif
