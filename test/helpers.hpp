#ifndef INC_METTLE_TEST_HELPERS_HPP
#define INC_METTLE_TEST_HELPERS_HPP

#include <mettle/matchers/core.hpp>
#include <mettle/suite/attributes.hpp>
#include <mettle/driver/filters_core.hpp>
#include <mettle/driver/test_name.hpp>

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

std::string to_printable(const test_name &test) {
  return test.full_name() + " (id=" + std::to_string(test.id) + ")";
}

template<typename T>
std::string to_printable(const basic_test_info<T> &test) {
  std::ostringstream ss;
  ss << "test_info(" << to_printable(test.name) << ", "
     << to_printable(test.attrs) << ")";
  return ss.str();
}

auto equal_attr_inst(attr_instance expected) {
  return make_matcher(
    std::move(expected),
    [](const attr_instance &actual, const attr_instance &expected) {
      return actual.attribute.name() == expected.attribute.name() &&
             actual.value == expected.value;
    }, ""
  );
}

inline auto equal_attributes(const attributes &expected) {
  return each(expected, equal_attr_inst);
}

auto equal_filter_result(filter_result expected) {
  return make_matcher(
    std::move(expected),
    [](const filter_result &actual, const filter_result &expected) {
      return actual.action == expected.action &&
             actual.message == expected.message;
    }, ""
  );
}

} // namespace mettle

#endif
