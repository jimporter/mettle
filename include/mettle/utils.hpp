#ifndef INC_METTLE_UTILS_HPP
#define INC_METTLE_UTILS_HPP

#include <stdexcept>

namespace mettle {

class expectation_error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

} // namespace mettle

#endif
