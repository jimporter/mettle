#ifndef INC_METTLE_DRIVER_LOG_FORMAT_HPP
#define INC_METTLE_DRIVER_LOG_FORMAT_HPP

#include <iostream>

#include "../../test_result.hpp"
#include "../test_name.hpp"

namespace mettle {

  std::ostream & operator <<(std::ostream &os, const test_failure &failure);
  std::ostream & operator <<(std::ostream &os, const test_name &name);

} // namespace mettle

#endif
