#ifndef INC_METTLE_DETAIL_HEADER_DRIVER_HPP
#define INC_METTLE_DETAIL_HEADER_DRIVER_HPP

#include <iostream>

#include "all_suites.hpp"
#include "../run_tests.hpp"
#include "../log/indent.hpp"
#include "../log/simple_summary.hpp"

int main() {
  using namespace mettle;

  indenting_ostream out(std::cout);

  log::simple_summary logger(out);
  run_tests(detail::all_suites(), logger, inline_test_runner);
  logger.summarize();
  return !logger.good();
}

#endif
