#ifndef INC_METTLE_DRIVER_LOG_FORMAT_HPP
#define INC_METTLE_DRIVER_LOG_FORMAT_HPP

#include <iostream>

#include "../../test_result.hpp"

namespace mettle {

  std::ostream & operator <<(std::ostream &os, const test_failure &failure);

} // namespace mettle

#endif
