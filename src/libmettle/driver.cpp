#include <iostream>
#include <vector>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/program_options.hpp>

#include <mettle/runner.hpp>
#include <mettle/log/multi_run.hpp>
#include <mettle/log/single_run.hpp>

#include "forked_test_runner.hpp"
#include "log_child.hpp"

namespace mettle {

using suites_list = std::vector<runnable_suite>;

namespace detail {
  suites_list all_suites;

  int real_main(int argc, const char *argv[]) {
    using namespace mettle;
    namespace opts = boost::program_options;

    opts::options_description generic("Generic options");
    generic.add_options()
      ("help,h", "show help")
    ;

    opts::options_description output("Output options");
    output.add_options()
      ("verbose,v", opts::value<unsigned int>()->implicit_value(1),
       "show verbose output")
      ("color,c", "show colored output")
      ("runs,n", opts::value<size_t>(), "number of test runs")
      ("show-terminal", "show terminal output for each test")
    ;

    opts::options_description child("Child options");
    child.add_options()
      ("timeout,t", opts::value<size_t>(), "timeout in ms")
      ("no-fork", "don't fork for each test")
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

    unsigned int verbosity = args.count("verbose") ?
      args["verbose"].as<unsigned int>() : 0;
    term::enabled(args.count("color"));
    bool fork_tests = !args.count("no-fork");
    bool show_terminal = args.count("show-terminal");

    if(show_terminal && verbosity < 2) {
      std::cerr << "--show-terminal requires verbosity >=2" << std::endl;
      return 1;
    }

    if(!fork_tests) {
      if(show_terminal) {
        std::cerr << "--show-terminal requires forking tests" << std::endl;
        return 1;
      }
      if(args.count("timeout")) {
        std::cerr << "--timeout requires forking tests" << std::endl;
        return 1;
      }
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
      namespace io = boost::iostreams;
      io::stream<io::file_descriptor_sink> fds(
        args["child"].as<int>(), io::never_close_handle
      );
      log::child logger(fds);
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
}

} // namespace mettle
