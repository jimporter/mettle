#include <iostream>
#include <vector>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/program_options.hpp>

#include <mettle/runner.hpp>
#include <mettle/log/summary.hpp>

#include "../cmd_line.hpp"
#include "cmd_parse.hpp"
#include "forked_test_runner.hpp"
#include "log_child.hpp"

namespace mettle {

using suites_list = std::vector<runnable_suite>;

namespace detail {
  suites_list all_suites;

  int real_main(int argc, const char *argv[]) {
    using namespace mettle;
    namespace opts = boost::program_options;

    unsigned int verbosity = 1;
    size_t runs = 1;
    filter_set filters;

    opts::options_description generic("Generic options");
    generic.add_options()
      ("help,h", "show help")
    ;

    opts::options_description output("Output options");
    output.add_options()
      ("verbose,v", opts::value(&verbosity)->implicit_value(2),
       "show verbose output")
      ("color,c", "show colored output")
      ("runs,n", opts::value(&runs), "number of test runs")
      ("show-terminal", "show terminal output for each test")
    ;

    opts::options_description child("Child options");
    child.add_options()
      ("timeout,t", opts::value(&runs), "timeout in ms")
      ("no-fork", "don't fork for each test")
      ("test,T", opts::value(&filters.by_name),
       "regex matching names of tests to run")
      ("attr,a", opts::value(&filters.by_attr), "attributes of tests to run")
    ;

    opts::options_description hidden("Hidden options");
    hidden.add_options()
      ("child", opts::value<int>(), "run this file as a child process")
    ;

    opts::variables_map args;
    try {
      opts::options_description all;
      all.add(generic).add(output).add(child).add(hidden);

      opts::store(opts::parse_command_line(argc, argv, all), args);
      opts::notify(args);
    } catch(const std::exception &e) {
      std::cerr << e.what() << std::endl;
      return 1;
    }

    if(args.count("help")) {
      opts::options_description displayed;
      displayed.add(generic).add(output).add(child);
      std::cout << displayed << std::endl;
      return 1;
    }

    bool fork_tests = !args.count("no-fork");
    test_runner runner;
    if(fork_tests) {
      METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds> timeout;
      if(args.count("timeout"))
        timeout.emplace(args["timeout"].as<size_t>());
      runner = forked_test_runner(timeout);
    }
    else {
      if(args.count("timeout")) {
        std::cerr << "--timeout requires forking tests" << std::endl;
        return 1;
      }
      runner = inline_test_runner;
    }

    if(args.count("child")) {
      // XXX: Complain if the user specified any output options.

      namespace io = boost::iostreams;
      io::stream<io::file_descriptor_sink> fds(
        args["child"].as<int>(), io::never_close_handle
      );
      log::child logger(fds);
      run_tests(detail::all_suites, logger, runner, filters);
      return 0;
    }

    if(runs == 0) {
      std::cerr << "no test runs, exiting" << std::endl;
      return 1;
    }

    term::enable(std::cout, args.count("color"));
    indenting_ostream out(std::cout);

    auto progress_log = make_progress_logger(
      out, verbosity, runs, args.count("show-terminal"), fork_tests
    );
    log::summary logger(out, progress_log.get());

    for(size_t i = 0; i != runs; i++)
      run_tests(detail::all_suites, logger, runner, filters);

    logger.summarize();
    return !logger.good();
  }
}

} // namespace mettle
