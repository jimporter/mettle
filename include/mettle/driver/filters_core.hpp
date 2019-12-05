#ifndef INC_METTLE_DRIVER_FILTERS_CORE_HPP
#define INC_METTLE_DRIVER_FILTERS_CORE_HPP

#include "../suite/attributes.hpp"
#include "../detail/algorithm.hpp"
#include "test_name.hpp"

namespace mettle {

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
    using namespace detail;
    for(const auto &attr : attrs) {
      if(attr.attribute.action() == test_action::skip)
        return {test_action::skip, stringify(joined(attr.value))};
    }
    return test_action::run;
  }

} // namespace mettle

#endif
