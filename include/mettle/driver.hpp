#ifndef INC_METTLE_DRIVER_HPP
#define INC_METTLE_DRIVER_HPP

#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include "glue.hpp"
#include "logger.hpp"
#include "runner.hpp"

namespace mettle {

using suites_list = std::vector<runnable_suite>;

namespace detail {
  suites_list all_suites;
}

template<typename Exception, typename ...Fixture>
struct basic_suite {
  template<typename F>
  basic_suite(const std::string &name, const F &f,
              suites_list &suites = detail::all_suites) {
    for (auto &i : make_basic_suites<Exception, Fixture...>(name, f))
      suites.push_back(std::move(i));
  }
};

template<typename ...T>
using suite = basic_suite<expectation_error, T...>;

} // namespace mettle

int main(int argc, const char *argv[]) {
  using namespace mettle;
  namespace opts = boost::program_options;

  opts::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "show help")
    ("verbose,v", opts::value<unsigned int>()->implicit_value(1),
     "show verbose output")
    ("color,c", "show colored output")
    ("runs", opts::value<size_t>(), "number of test runs")
    ("no-fork", "don't fork for each test")
    ("show-terminal", "show terminal output for each test")
  ;

  opts::variables_map args;
  opts::store(opts::parse_command_line(argc, argv, desc), args);
  opts::notify(args);

  if(args.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  unsigned int verbosity = args.count("verbose") ?
    args["verbose"].as<unsigned int>() : 0;
  term::colors_enabled = args.count("color");
  bool fork_tests = !args.count("no-fork");
  bool show_terminal = args.count("show-terminal");

  if(show_terminal && verbosity < 2) {
    std::cerr << "--show-terminal requires verbosity >=2" << std::endl;
    return 1;
  }
  if(show_terminal && !fork_tests) {
    std::cerr << "--show-terminal requires forking tests" << std::endl;
    return 1;
  }

  verbose_logger vlog(std::cout, verbosity, show_terminal);

  if(args.count("runs")) {
    size_t runs = args["runs"].as<size_t>();
    if(runs == 0) {
      std::cout << "no test runs, exiting" << std::endl;
      return 1;
    }

    multi_run_logger logger(vlog);
    for(size_t i = 0; i < runs; i++)
      run_tests(detail::all_suites, logger, fork_tests);
    logger.summarize();

    return logger.failures();
  }
  else {
    single_run_logger logger(vlog);
    run_tests(detail::all_suites, logger, fork_tests);
    logger.summarize();

    return logger.failures();
  }
}

#endif
