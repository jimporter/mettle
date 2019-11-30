#ifndef INC_METTLE_DRIVER_TEST_NAME_HPP
#define INC_METTLE_DRIVER_TEST_NAME_HPP

#include <sstream>
#include <string>
#include <vector>

#include "../test_uid.hpp"

namespace mettle {

  struct test_name {
    std::vector<std::string> suites;
    std::string test;
    test_uid id;

    std::string full_name() const {
      std::ostringstream ss;
      for(const auto &i : suites)
        ss << i << " > ";
      ss << test;
      return ss.str();
    }
  };

  inline bool operator ==(const test_name &lhs, const test_name &rhs) {
    return lhs.id == rhs.id;
  }
  inline bool operator !=(const test_name &lhs, const test_name &rhs) {
    return lhs.id != rhs.id;
  }
  inline bool operator <(const test_name &lhs, const test_name &rhs) {
    return lhs.id < rhs.id;
  }
  inline bool operator <=(const test_name &lhs, const test_name &rhs) {
    return lhs.id <= rhs.id;
  }
  inline bool operator >(const test_name &lhs, const test_name &rhs) {
    return lhs.id > rhs.id;
  }
  inline bool operator >=(const test_name &lhs, const test_name &rhs) {
    return lhs.id >= rhs.id;
  }

} // namespace mettle

#endif
