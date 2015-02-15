#ifndef INC_METTLE_TEST_POSIX_ERRNO_HPP
#define INC_METTLE_TEST_POSIX_ERRNO_HPP

#include <mettle/matchers/core.hpp>
#include "../../src/libmettle/posix/err_string.hpp"

class equal_errno : public mettle::matcher_tag {
public:
  equal_errno(int errnum) : expected_(errnum) {}

  template<typename T>
  mettle::match_result operator ()(const T &) const {
    int errnum = errno;
    return {errnum == expected_, posix::err_string(errnum)};
  }

  std::string desc() const {
    return mettle::posix::err_string(expected_);
  }
private:
  int expected_;
};

#endif
