#ifndef INC_METTLE_DRIVER_HPP
#define INC_METTLE_DRIVER_HPP

#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include "glue.hpp"
#include "runner.hpp"
#include "log/multi_run.hpp"
#include "log/single_run.hpp"
#include "log/child.hpp"

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
    for(auto &&i : make_basic_suites<Exception, Fixture...>(name, f))
      suites.push_back(std::move(i));
  }
};

template<typename Exception, typename ...Fixture>
struct skip_basic_suite {
  template<typename F>
  skip_basic_suite(const std::string &name, const F &f,
                   suites_list &suites = detail::all_suites) {
    for(auto &&i : make_skip_basic_suites<Exception, Fixture...>(name, f))
      suites.push_back(std::move(i));
  }
};

template<typename ...T>
using suite = basic_suite<expectation_error, T...>;

template<typename ...T>
using skip_suite = skip_basic_suite<expectation_error, T...>;

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
    ("runs,n", opts::value<size_t>(), "number of test runs")
    ("timeout,t", opts::value<size_t>(), "timeout in ms")
    ("no-fork", "don't fork for each test")
    ("show-terminal", "show terminal output for each test")
    ("child", "run this file as a child process")
  ;

  opts::variables_map args;
  try {
    opts::store(opts::parse_command_line(argc, argv, desc), args);
    opts::notify(args);
  } catch(const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

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
  if(args.count("timeout") && !fork_tests) {
    std::cerr << "--timeout requires forking tests" << std::endl;
    return 1;
  }

  test_runner runner;
  if(fork_tests) {
    METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds> timeout;
    if(args.count("timeout"))
      timeout.emplace(args["timeout"].as<size_t>());
    runner = forked_test_runner(timeout);
  }
  else {
    runner = inline_test_runner;
  }

  if(args.count("child")) {
    log::child logger(std::cout);
    run_tests(detail::all_suites, logger, runner);
    return 0;
  }

  log::verbose vlog(std::cout, verbosity, show_terminal);

  if(args.count("runs")) {
    size_t runs = args["runs"].as<size_t>();
    if(runs == 0) {
      std::cout << "no test runs, exiting" << std::endl;
      return 1;
    }

    log::multi_run logger(vlog);
    for(size_t i = 0; i < runs; i++)
      run_tests(detail::all_suites, logger, runner);
    logger.summarize();

    return !logger.good();
  }
  else {
    log::single_run logger(vlog);
    run_tests(detail::all_suites, logger, runner);
    logger.summarize();

    return !logger.good();
  }
}

#endif
