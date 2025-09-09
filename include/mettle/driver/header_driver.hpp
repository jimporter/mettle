#ifndef INC_METTLE_DRIVER_HEADER_DRIVER_HPP
#define INC_METTLE_DRIVER_HEADER_DRIVER_HPP

#include <iostream>

#define LIBMETTLE_STATIC

#include "run_tests.hpp"
#include "log/indent.hpp"
#include "log/simple_summary.hpp"
#include "../suite/detail/all_suites.hpp"

int main() {
  using namespace mettle;

  indenting_ostream out(std::cout);

  log::simple_summary logger(out);
  run_tests(detail::all_suites, logger, inline_test_runner);
  logger.summarize();
  return !logger.good();
}

#endif
