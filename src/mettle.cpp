#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include <mettle/log/summary.hpp>

#include "libmettle/cmd_line.hpp"
#include "file_runner.hpp"

namespace mettle {

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

}

int main(int argc, const char *argv[]) {
  using namespace mettle;
  namespace opts = boost::program_options;

  unsigned int verbosity = 1;
  size_t runs = 1;
  std::vector<std::string> files;

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
    ("timeout,t", opts::value<size_t>(), "timeout in ms")
    ("no-fork", "don't fork for each test")
    ("test,T", opts::value< std::vector<std::string> >(),
     "regex matching names of tests to run")
    ("attr,a", opts::value< std::vector<std::string> >(),
     "attributes of tests to run")
  ;

  opts::options_description hidden("Hidden options");
  hidden.add_options()
    ("file", opts::value(&files), "input file")
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

  if(files.empty()) {
    std::cerr << "no inputs specified" << std::endl;
    return 1;
  }

  if(runs == 0) {
    std::cerr << "no test runs, exiting" << std::endl;
    return 1;
  }

  term::enable(std::cout, args.count("color"));
  indenting_ostream out(std::cout);

  auto progress_log = make_progress_logger(
    out, verbosity, runs, args.count("show-terminal"), !args.count("no-fork")
  );
  log::summary logger(out, progress_log.get());

  for(size_t i = 0; i != runs; i++)
    run_test_files(files, logger, child_args);

  logger.summarize();
  return !logger.good();
}
