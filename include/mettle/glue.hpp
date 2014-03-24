#ifndef INC_METTLE_GLUE_HPP
#define INC_METTLE_GLUE_HPP

#include "suite.hpp"
#include "matchers.hpp"

namespace mettle {

template<typename ...T>
using suite = basic_suite<expectation_error, T...>;

} // namespace mettle

#endif
