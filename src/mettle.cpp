#include <iostream>

#include <boost/program_options.hpp>

#include "file_runner.hpp"
#include <mettle/log/multi_run.hpp>
#include <mettle/log/single_run.hpp>

template<typename Char>
std::vector<std::basic_string<Char>> filter_options(
  const boost::program_options::basic_parsed_options<Char> &parsed,
  const boost::program_options::options_description &desc
) {
  std::vector<std::basic_string<Char>> filtered;
  for(auto &&option : parsed.options) {
    if(desc.find_nothrow(option.string_key, false)) {
      auto &&tokens = option.original_tokens;
      filtered.insert(filtered.end(), tokens.begin(), tokens.end());
    }
  }
  return filtered;
}

int main(int argc, const char *argv[]) {
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
    ("test,T", opts::value< std::vector<std::string> >(),
     "regex matching names of tests to run")
    ("attr,a", opts::value< std::vector<std::string> >(),
     "attributes of tests to run")
  ;

  opts::options_description hidden("Hidden options");
  hidden.add_options()
    ("file", opts::value< std::vector<std::string> >(), "input file")
  ;

  opts::positional_options_description pos;
  pos.add("file", -1);

  opts::variables_map args;
  std::vector<std::string> child_args;
  try {
    opts::options_description all;
    all.add(generic).add(output).add(child).add(hidden);
    auto parsed = opts::command_line_parser(argc, argv)
      .options(all).positional(pos).run();

    opts::store(parsed, args);
    opts::notify(args);
    child_args = filter_options(parsed, child);
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

  if(!args.count("file")) {
    std::cerr << "no inputs specified" << std::endl;
    return 1;
  }

  unsigned int verbosity = args.count("verbose") ?
    args["verbose"].as<unsigned int>() : 0;
  term::enable(std::cout, args.count("color"));
  bool show_terminal = args.count("show-terminal");

  if(show_terminal && verbosity < 2) {
    std::cerr << "--show-terminal requires verbosity >=2" << std::endl;
    return 1;
  }

  if(args.count("no-fork")) {
    if(show_terminal) {
      std::cerr << "--show-terminal requires forking tests" << std::endl;
      return 1;
    }
    if(args.count("timeout")) {
      std::cerr << "--timeout requires forking tests" << std::endl;
      return 1;
    }
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
      run_test_files(files, logger, child_args);
    logger.summarize();

    return !logger.good();
  }
  else {
    log::single_run logger(vlog);
    run_test_files(files, logger, child_args);
    logger.summarize();

    return !logger.good();
  }
}
