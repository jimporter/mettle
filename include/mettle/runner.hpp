#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

#include <iostream>
#include <boost/program_options.hpp>

#include "term.hpp"

namespace mettle {

using suites_list = std::vector<runnable_suite>;

namespace detail {
  suites_list all_suites;

  template<typename T>
  std::string join(T begin, T end, const std::string &delim) {
    if(begin == end)
      return "";

    std::stringstream s;
    s << *begin;
    for(++begin; begin != end; ++begin)
      s << delim << *begin;
    return s.str();
  }

  struct test_results {
    test_results() : passes(0), skips(0), total(0) {}

    struct failure {
      std::vector<std::string> suites;
      std::string test, message;

      std::string full_name() const {
        std::stringstream s;
        s << join(suites.begin(), suites.end(), " > ") << " > " << test;
        return s.str();
      }
    };

    std::vector<failure> failures;
    size_t passes, skips, total;
  };

  void run_tests_impl(test_results &results, const suites_list &suites,
                      bool verbose, std::vector<std::string> &parents) {
    using namespace term;
    const std::string indent(parents.size() * 2, ' ');

    for(auto &suite : suites) {
      parents.push_back(suite.name());

      if(verbose) {
        std::cout << indent << format(sgr::bold) << suite.name()
                  << reset() << std::endl;
      }

      for(auto &test : suite) {
        results.total++;

        if(verbose)
          std::cout << indent << "  " << test.name << " " << std::flush;
        if(test.skip) {
          results.skips++;
          if(verbose) {
            std::cout << format(sgr::bold, fg(color::blue)) << "SKIPPED"
                      << reset() << std::endl;
          }
          continue;
        }

        auto result = test.function();
        if(result.passed) {
          results.passes++;
          if(verbose) {
            std::cout << format(sgr::bold, fg(color::green)) << "PASSED"
                      << reset() << std::endl;
          }
        }
        else {
          results.failures.push_back({parents, test.name, result.message});
          if(verbose) {
            std::cout << format(sgr::bold, fg(color::red)) << "FAILED"
                      << reset() << ": " << result.message << std::endl;
          }
        }
      }

      if(verbose)
        std::cout << std::endl;

      run_tests_impl(results, suite.subsuites(), verbose, parents);
      parents.pop_back();
    }
  }

  void run_tests(test_results &results, bool verbose) {
    std::vector<std::string> parents;
    run_tests_impl(results, all_suites, verbose, parents);
  }
}

template<typename ...T, typename F>
inline auto make_suite(const std::string &name, F &&f) {
  return make_basic_suite<expectation_error, T..., F>(name, std::forward<F>(f));
}

template<typename Exception, typename ...T>
struct basic_suite {
public:
  template<typename F>
  basic_suite(const std::string &name, F &&f,
              suites_list &suites = detail::all_suites) {
    suites.push_back(
      make_basic_suite<Exception, T...>(name, std::forward<F>(f))
    );
  }
};

template<typename ...T>
using suite = basic_suite<expectation_error, T...>;

} // namespace mettle

int main(int argc, const char *argv[]) {
  using namespace term;
  using namespace mettle::detail;
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

  colors_enabled = args.count("color");
  bool verbose = args.count("verbose");

  test_results results;
  run_tests(results, verbose);

  std::cout << format(sgr::bold) << results.passes << "/" << results.total
            << " tests passed";
  if(results.skips)
    std::cout << " (" << results.skips << " skipped)";
  std::cout << reset() << std::endl;

  for(auto &i : results.failures) {
    std::cout << "  " << i.full_name() << " "
              << format(sgr::bold, fg(color::red)) << "FAILED" << reset()
              << ": " << i.message << std::endl;
  }

  return results.failures.size();
}

#endif
