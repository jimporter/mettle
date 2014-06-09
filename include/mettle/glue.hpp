#ifndef INC_METTLE_GLUE_HPP
#define INC_METTLE_GLUE_HPP

#include "suite.hpp"
#include "error.hpp"

namespace mettle {

template<typename ...T, typename F>
inline runnable_suite make_suite(const std::string &name, F &&f) {
  return make_basic_suite<expectation_error, T...>(name, std::forward<F>(f));
}

} // namespace mettle

#endif
