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
#include <mettle/suite/compiled_suite.hpp>

#include "forked_test_runner.hpp"

namespace mettle {

namespace {
  struct all_options : generic_options, output_options, child_options {
    METTLE_OPTIONAL_NS::optional<int> child_fd;
  };
}

namespace detail {

  int drive_tests(int argc, const char *argv[], const suites_list &suites) {
    using namespace mettle;
    namespace opts = boost::program_options;

    all_options args;
    auto generic = make_generic_options(args);
    auto output = make_output_options(args);
    auto child = make_child_options(args);

    opts::options_description hidden("Hidden options");
    hidden.add_options()
      ("child", opts::value(&args.child_fd), "run this file as a child process")
    ;

    opts::variables_map vm;
    try {
      opts::options_description all;
      all.add(generic).add(output).add(child).add(hidden);

      opts::store(opts::parse_command_line(argc, argv, all), vm);
      opts::notify(vm);
    } catch(const std::exception &e) {
      std::cerr << e.what() << std::endl;
      return 1;
    }

    if(args.show_help) {
      opts::options_description displayed;
      displayed.add(generic).add(output).add(child);
      std::cout << displayed << std::endl;
      return 1;
    }

    test_runner runner;
    if(args.no_fork) {
      if(args.timeout) {
        std::cerr << "--timeout requires forking tests" << std::endl;
        return 1;
      }
      runner = inline_test_runner;
    }
    else {
      runner = forked_test_runner(args.timeout);
    }

    if(args.child_fd) {
      if(auto output_opt = has_option(output, vm)) {
        using namespace opts::command_line_style;
        std::cerr << output_opt->canonical_display_name(allow_long)
                  << " can't be used with --child" << std::endl;
        return 1;
      }

      namespace io = boost::iostreams;
      io::stream<io::file_descriptor_sink> fds(
        *args.child_fd, io::never_close_handle
      );
      log::child logger(fds);
      run_tests(suites, logger, runner, args.filters);
      return 0;
    }

    if(args.no_fork && args.show_terminal) {
      std::cerr << "--show-terminal requires forking tests" << std::endl;
      return 1;
    }

    if(args.runs == 0) {
      std::cerr << "no test runs, exiting" << std::endl;
      return 1;
    }

    term::enable(std::cout, args.color);
    indenting_ostream out(std::cout);

    auto progress_log = make_progress_logger(out, args);
    log::summary logger(out, progress_log.get(), args.show_time,
                        args.show_terminal);

    for(size_t i = 0; i != args.runs; i++)
      run_tests(suites, logger, runner, args.filters);

    logger.summarize();
    return !logger.good();
  }
}

} // namespace mettle
