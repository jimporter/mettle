#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include <mettle/driver/cmd_line.hpp>
#include <mettle/driver/log/summary.hpp>
#include <mettle/driver/log/term.hpp>

#include "run_test_files.hpp"

namespace mettle {

namespace {
  struct all_options : generic_options, output_options, child_options {
    std::vector<std::string> files;
  };

  const char program_name[] = "mettle";
  void report_error(const std::string &message) {
    std::cerr << program_name << ": " << message << std::endl;
  }
}

} // namespace mettle

int main(int argc, const char *argv[]) {
  using namespace mettle;
  namespace opts = boost::program_options;

  all_options args;
  auto generic = make_generic_options(args);
  auto output = make_output_options(args);
  auto child = make_child_options(args);

  opts::options_description hidden("Hidden options");
  hidden.add_options()
    ("file", opts::value(&args.files), "input file")
  ;
  opts::positional_options_description pos;
  pos.add("file", -1);

  std::vector<std::string> child_args;
  try {
    opts::options_description all;
    all.add(generic).add(output).add(child).add(hidden);
    auto parsed = opts::command_line_parser(argc, argv)
      .options(all).positional(pos).run();

    opts::variables_map vm;
    opts::store(parsed, vm);
    opts::notify(vm);
    child_args = filter_options(parsed, child);
  } catch(const std::exception &e) {
    mettle::report_error(e.what());
    return 2;
  }

  if(args.show_help) {
    opts::options_description displayed;
    displayed.add(generic).add(output).add(child);
    std::cout << displayed << std::endl;
    return 0;
  }

  if(args.no_fork && args.show_terminal) {
    mettle::report_error("--show-terminal requires forking tests");
    return 2;
  }

  if(args.files.empty()) {
    mettle::report_error("no inputs specified");
    return 1;
  }

  if(args.runs == 0) {
    mettle::report_error("no test runs, exiting");
    return 1;
  }

  try {
    term::enable(std::cout, args.color);
    indenting_ostream out(std::cout);

    auto progress_log = make_progress_logger(out, args);
    log::summary logger(out, progress_log.get(), args.show_time,
                        args.show_terminal);

    for(size_t i = 0; i != args.runs; i++)
      run_test_files(args.files, logger, child_args);

    logger.summarize();
    return !logger.good();
  }
  catch(const std::exception &e) {
    mettle::report_error(e.what());
    return 3;
  }
}
