#include <cstdint>
#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include <mettle/driver/cmd_line.hpp>
#include <mettle/driver/exit_code.hpp>
#include <mettle/driver/log/summary.hpp>
#include <mettle/driver/log/term.hpp>

#include "run_test_files.hpp"

namespace mettle {

  namespace {
    struct all_options : generic_options, driver_options, output_options {
      std::vector<test_command> files;
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

  auto factory = make_logger_factory();

  all_options args;
  auto generic = make_generic_options(args);
  auto driver = make_driver_options(args);
  auto output = make_output_options(args, factory);

  opts::options_description hidden("Hidden options");
  hidden.add_options()
    ("input-file", opts::value(&args.files), "input file")
  ;
  opts::positional_options_description pos;
  pos.add("input-file", -1);

  std::vector<std::string> child_args;
  try {
    opts::options_description all;
    all.add(generic).add(driver).add(output).add(hidden);
    auto parsed = opts::command_line_parser(argc, argv)
      .options(all).positional(pos).run();

    opts::variables_map vm;
    opts::store(parsed, vm);
    opts::notify(vm);
    child_args = filter_options(parsed, driver);
  } catch(const std::exception &e) {
    report_error(e.what());
    return exit_code::bad_args;
  }

  if(args.show_help) {
    opts::options_description displayed;
    displayed.add(generic).add(driver).add(output);
    std::cout << displayed << std::endl;
    return exit_code::success;
  } else if(args.show_version) {
    std::cout << "mettle " << METTLE_VERSION << std::endl;
    return exit_code::success;
  }

  if(args.files.empty()) {
    report_error("no inputs specified");
    return exit_code::no_inputs;
  }

  if(args.runs == 0) {
    report_error("no test runs, exiting");
    return exit_code::no_inputs;
  }

  try {
    term::enable(std::cout, color_enabled(args.color));
    indenting_ostream out(std::cout);

    log::summary logger(
      out, factory.make(args.output, out, args), args.show_time,
      args.show_terminal
    );
    for(std::size_t i = 0; i != args.runs; i++)
      run_test_files(args.files, logger, child_args);

    logger.summarize();
    return logger.good() ? exit_code::success : exit_code::failure;
  } catch(const std::out_of_range &e) {
    report_error("unknown output format \"" + args.output + "\"");
    return exit_code::bad_args;
  } catch(const std::exception &e) {
    report_error(e.what());
    return exit_code::unknown_error;
  }
}
