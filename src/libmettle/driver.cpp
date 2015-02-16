#include <cstdint>
#include <iostream>
#include <vector>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/program_options.hpp>

#include <mettle/driver/run_tests.hpp>
#include <mettle/driver/cmd_line.hpp>
#include <mettle/driver/log/child.hpp>
#include <mettle/driver/log/summary.hpp>
#include <mettle/driver/log/term.hpp>
#include <mettle/driver/detail/export.hpp>
#include <mettle/suite/compiled_suite.hpp>

#ifndef _WIN32
#  include <mettle/driver/posix/subprocess.hpp>
#  include "posix/subprocess_test_runner.hpp"
namespace platform = mettle::posix;
#else
#  include <mettle/driver/windows/subprocess.hpp>
#  include "windows/subprocess_test_runner.hpp"
namespace platform = mettle::windows;
#endif

namespace mettle {

namespace {
  struct all_options : generic_options, driver_options, output_options {
#ifndef _WIN32
    METTLE_OPTIONAL_NS::optional<int> output_fd;
#else
    METTLE_OPTIONAL_NS::optional<HANDLE> output_fd;
    METTLE_OPTIONAL_NS::optional<test_uid> test_id;
    METTLE_OPTIONAL_NS::optional<HANDLE> log_fd;
#endif
    bool no_subproc = false;
  };

  void report_error(const std::string &program_name,
                    const std::string &message) {
    std::cerr << program_name << ": " << message << std::endl;
  }
}

namespace detail {

  METTLE_PUBLIC int
  drive_tests(int argc, const char *argv[], const suites_list &suites) {
    using namespace mettle;
    using namespace platform;
    namespace opts = boost::program_options;

    auto factory = make_logger_factory();

    all_options args;
    auto generic = make_generic_options(args);
    auto driver = make_driver_options(args);
    auto output = make_output_options(args, factory);

    driver.add_options()
      ("no-subproc", opts::value(&args.no_subproc)->zero_tokens(),
       "don't create a subprocess for each test")
    ;

    opts::options_description hidden("Hidden options");
    hidden.add_options()
      ("output-fd", opts::value(&args.output_fd),
       "pipe the results to this file descriptor")
#ifdef _WIN32
      ("test-id", opts::value(&args.test_id), "internal id of a test to run")
      ("log-fd", opts::value(&args.log_fd), "HANDLE to log pipe")
#endif
    ;

    opts::variables_map vm;
    try {
      opts::options_description all;
      opts::positional_options_description pos;
      all.add(generic).add(driver).add(output).add(hidden);
      auto parsed = opts::command_line_parser(argc, argv)
        .options(all).positional(pos).run();

      opts::store(parsed, vm);
      opts::notify(vm);
    } catch(const std::exception &e) {
      report_error(argv[0], e.what());
      return 2;
    }

    if(args.show_help) {
      opts::options_description displayed;
      displayed.add(generic).add(driver).add(output);
      std::cout << displayed << std::endl;
      return 0;
    }

#ifdef _WIN32
    if(args.test_id || args.log_fd) {
      if(!args.test_id || !args.log_fd) {
        report_error(
          argv[0], "--test-id and --log-fd must be specified together"
        );
        return 3;
      }
      auto test = find_test(suites, *args.test_id);
      if(!test) {
        report_error(argv[0], "unable to find test");
        return 3;
      }
      return run_single_test(*test, *args.log_fd) ? 0 : 1;
    }
#endif

    test_runner runner;
    if(args.no_subproc) {
      if(args.timeout) {
        report_error(
          argv[0], "--timeout requires running tests in subprocesses"
        );
        return 2;
      }
      runner = inline_test_runner;
    }
    else {
      runner = subprocess_test_runner(args.timeout);
    }

    if(args.output_fd) {
      if(auto output_opt = has_option(output, vm)) {
        using namespace opts::command_line_style;
        report_error(argv[0], output_opt->canonical_display_name(allow_long) +
                              " can't be used with --output-fd");
        return 2;
      }

      make_fd_private(*args.output_fd);
      namespace io = boost::iostreams;
      io::stream<io::file_descriptor_sink> fds(
        *args.output_fd, io::never_close_handle
      );
      log::child logger(fds);
      run_tests(suites, logger, runner, args.filters);
      return 0;
    }

    if(args.no_subproc && args.show_terminal) {
      report_error(
        argv[0], "--show-terminal requires running tests in subprocesses"
      );
      return 2;
    }

    if(args.runs == 0) {
      report_error(argv[0], "no test runs, exiting");
      return 1;
    }

    try {
      term::enable(std::cout, color_enabled(args.color));
      indenting_ostream out(std::cout);

      log::summary logger(
        out, factory.make(args.output, out, args), args.show_time,
        args.show_terminal
      );
      for(std::size_t i = 0; i != args.runs; i++)
        run_tests(suites, logger, runner, args.filters);

      logger.summarize();
      return !logger.good();
    }
    catch(const std::out_of_range &e) {
      report_error(argv[0], "unknown output format \"" + args.output + "\"");
      return 3;
    }
    catch(const std::exception &e) {
      report_error(argv[0], e.what());
      return 3;
    }
  }
}

} // namespace mettle
