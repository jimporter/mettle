#ifndef INC_METTLE_TEST_RESULT_HPP
#define INC_METTLE_TEST_RESULT_HPP

#include <optional>
#include <string>

namespace mettle {

  struct test_failure {
    std::string message;
  };

  using test_result = std::optional<test_failure>;

} // namespace mettle

#endif
