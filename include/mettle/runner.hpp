#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

#include <iostream>
#include <boost/program_options.hpp>

namespace mettle {

enum class color {
  reset = 0,
  black = 30,
  red = 31,
  green = 32,
  yellow = 33,
  blue = 34,
  magenta = 35,
  cyan = 36,
  white = 37
};

bool colors_enabled = false;
inline std::ostream & operator <<(std::ostream &o, const color &c) {
  if(colors_enabled) {
    if(c == color::reset)
      return o << "\e[0m";
    else
      return o << "\e[0;" << static_cast<size_t>(c) << "m";
  }
  else
    return o;
}

template<typename ...T, typename F>
inline auto make_suite(const std::string &name, F &&f) {
  return make_basic_suite<expectation_error, T..., F>(name, std::forward<F>(f));
}

using suites_list = std::vector<std::shared_ptr<runnable_suite>>;
suites_list all_suites;

template<typename Exception, typename ...T>
struct basic_suite {
public:
  template<typename F>
  basic_suite(const std::string &name, F &&f,
              suites_list &suites = all_suites) {
    suites.push_back(
      make_basic_suite<Exception, T...>(name, std::forward<F>(f))
    );
  }
};

template<typename ...T>
using suite = basic_suite<expectation_error, T...>;

} // namespace mettle

int main(int argc, const char *argv[]) {
  using color = mettle::color;
  namespace opts = boost::program_options;

  opts::options_description desc("Allowed options");
  desc.add_options()
    ("help,h", "show help")
    ("verbose", "show verbose output")
    ("color", "show colored output")
  ;

  opts::variables_map args;
  opts::store(opts::parse_command_line(argc, argv, desc), args);
  opts::notify(args);

  if(args.count("help")) {
    std::cout << desc << std::endl;
    return 1;
  }

  mettle::colors_enabled = args.count("color");
  bool verbose = args.count("verbose");

  struct failure {
    std::string suite, test, message;
  };
  std::vector<failure> fail_info;
  size_t passes = 0, skips = 0, total_tests = 0;

  for(auto &suite : mettle::all_suites) {
    if(verbose)
      std::cout << suite->name() << std::endl;

    for(auto &test : *suite) {
      total_tests++;

      if(verbose)
        std::cout << "  " << test.name << " " << std::flush;
      if(test.skip) {
        skips++;
        if(verbose)
          std::cout << color::blue << "SKIPPED" << color::reset << std::endl;
        continue;
      }

      auto result = test.function();
      if(result.passed) {
        passes++;
        if(verbose)
          std::cout << color::green << "PASSED" << color::reset << std::endl;
      }
      else {
        fail_info.push_back({ suite->name(), test.name, result.message });
        if(verbose) {
          std::cout << color::red << "FAILED" << color::reset << ": "
                    << result.message << std::endl;
        }
      }
    }

    if(verbose)
      std::cout << std::endl;
  }

  std::cout << passes << "/" << total_tests << " tests passed";
  if(skips)
    std::cout << " (" << skips << " skipped)";
  std::cout << std::endl;

  for(auto &i : fail_info)
    std::cout << "  " << i.suite << " > " << i.test << " " << color::red
              << "FAILED" << color::reset << ": " << i.message << std::endl;

  return fail_info.size();
}

#endif
