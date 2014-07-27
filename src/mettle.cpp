#include <iostream>

#include <boost/program_options.hpp>

#include <mettle/file_runner.hpp>
#include <mettle/log/multi_run.hpp>
#include <mettle/log/single_run.hpp>

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
    ("show-terminal", "show terminal output for each test")
    ("file", opts::value< std::vector<std::string> >(), "input file")
  ;

  opts::positional_options_description pos;
  pos.add("file", -1);

  opts::variables_map args;
  try {
    opts::store(opts::command_line_parser(argc, argv)
                .options(desc).positional(pos).run(), args);
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
  bool show_terminal = args.count("show-terminal");

  if(show_terminal && verbosity < 2) {
    std::cerr << "--show-terminal requires verbosity >=2" << std::endl;
    return 1;
  }

  if(!args.count("file")) {
    std::cerr << "no inputs specified" << std::endl;
    return 1;
  }

  auto files = args["file"].as< std::vector<std::string> >();
  log::verbose vlog(std::cout, verbosity, show_terminal);

  if(args.count("runs")) {
    size_t runs = args["runs"].as<size_t>();
    if(runs == 0) {
      std::cout << "no test runs, exiting" << std::endl;
      return 1;
    }

    log::multi_run logger(vlog);
    for(size_t i = 0; i < runs; i++)
      run_test_files(files, logger);
    logger.summarize();

    return logger.failures();
  }
  else {
    log::single_run logger(vlog);
    run_test_files(files, logger);
    logger.summarize();

    return logger.failures();
  }
}
