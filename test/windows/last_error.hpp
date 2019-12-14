#ifndef INC_METTLE_TEST_WINDOW_LAST_ERROR_HPP
#define INC_METTLE_TEST_WINDOW_LAST_ERROR_HPP

#include <mettle/matchers/core.hpp>
#include "../../src/err_string.hpp"

class equal_last_error : public mettle::matcher_tag {
public:
  equal_last_error(DWORD errnum) : expected_(errnum) {}

  template<typename T>
  mettle::match_result operator ()(const T &) const {
    int errnum = GetLastError();
    return {errnum == expected_, mettle::err_string(errnum)};
  }

  std::string desc() const {
    return to_printable(mettle::err_string(expected_));
  }
private:
  DWORD expected_;
};

#endif
