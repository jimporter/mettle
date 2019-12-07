#ifndef INC_METTLE_TEST_HELPERS_HPP
#define INC_METTLE_TEST_HELPERS_HPP

#include <mettle/matchers/core.hpp>
#include <mettle/suite/attributes.hpp>
#include <mettle/driver/filters.hpp>
#include <mettle/driver/test_name.hpp>

#include <boost/any.hpp>

namespace mettle {

  std::string to_printable(const attr_instance &attr) {
    std::ostringstream ss;
    ss << attr.attribute.name();
    if(!attr.value.empty()) {
      ss << "(" << detail::joined(attr.value, [](auto &&i) {
        return to_printable(i);
      }) << ")";
    }
    return ss.str();
  }

// MSVC doesn't understand [[noreturn]], so just ignore the warning here.
#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(push)
#  pragma warning(disable:4715)
#endif

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
    default:
      assert(false && "unexpected value");
    }
  }

#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(pop)
#endif

  std::string to_printable(const attr_filter_item &item) {
    return "filter_item(" + to_printable(item.attribute) + ")";
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
    return basic_matcher(
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
    return basic_matcher(
      std::move(expected),
      [](const filter_result &actual, const filter_result &expected) {
        return actual.action == expected.action &&
               actual.message == expected.message;
      }, ""
    );
  }

  auto equal_test_info(const std::string &name, const attributes &attrs) {
    std::ostringstream ss;
    ss << "test_info(" << to_printable(name) << ", " << to_printable(attrs)
       << ")";

    return basic_matcher(
      [name = equal_to(name), attrs = equal_attributes(attrs)]
      (const auto &actual) -> bool {
        return name(actual.name) && attrs(actual.attrs);
      }, ss.str()
    );
  }

  template<typename T>
  inline auto equal_test_info(const T &expected) {
    return equal_test_info(expected.name, expected.attrs);
  }

  template<typename Type, typename T>
  auto match_any(T &&thing) {
    return basic_matcher(
      ensure_matcher(std::forward<T>(thing)),
      [](const boost::any &actual, auto &&matcher) -> match_result {
        using ValueType = typename std::remove_reference<Type>::type;
        auto value = boost::any_cast<ValueType>(&actual);
        if(!value)
          return {
            false, "value not of type \"" + type_name<ValueType>() + "\""
          };
        return matcher(*value);
      }, ""
    );
  }

  template<typename T>
  auto any_equal(T &&thing) {
    return match_any<T>(std::forward<T>(thing));
  }

} // namespace mettle

#endif
