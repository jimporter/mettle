#ifndef INC_METTLE_TEST_INFO_HPP
#define INC_METTLE_TEST_INFO_HPP

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace mettle {

struct test_name {
  std::vector<std::string> suites;
  std::string test;
  uint64_t id;

  std::string full_name() const {
    std::stringstream s;
    for(const auto &i : suites)
      s << i << " > ";
    s << test;
    return s.str();
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

enum class test_action {
  run,
  skip,
  hide,
  indeterminate
};

} // namespace mettle

#endif
