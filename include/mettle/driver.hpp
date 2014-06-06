#ifndef INC_METTLE_DRIVER_HPP
#define INC_METTLE_DRIVER_HPP

#include <iostream>
#include <boost/program_options.hpp>

#include "term.hpp"
#include "runner.hpp"

namespace mettle {

using suites_list = std::vector<runnable_suite>;

namespace detail {
  suites_list all_suites;

  class single_run_logger : public test_logger {
  public:
    single_run_logger(bool verbose) : verbose_(verbose) {}

    void start_suite(const std::vector<std::string> &suites) {
      using namespace term;
      if(verbose_) {
        const std::string indent((suites.size() - 1) * 2, ' ');
        std::cout << indent << format(sgr::bold) << suites.back()
                  << reset() << std::endl;
      }
    }

    void end_suite(const std::vector<std::string> &) {
      if(verbose_)
        std::cout << std::endl;
    }

    void start_test(const test_name &test) {
      if(verbose_) {
        const std::string indent(test.suites.size() * 2, ' ');
        std::cout << indent << test.test << " " << std::flush;
      }
    }

    void passed_test(const test_name &) {
      using namespace term;
      if(verbose_) {
        std::cout << format(sgr::bold, fg(color::green)) << "PASSED" << reset()
                  << std::endl;
      }
    }

    void skipped_test(const test_name &) {
      using namespace term;
      if(verbose_) {
        std::cout << format(sgr::bold, fg(color::blue)) << "SKIPPED" << reset()
                  << std::endl;
      }
    }

    void failed_test(const test_name &, const std::string &message) {
      using namespace term;
      if(verbose_) {
        std::cout << format(sgr::bold, fg(color::red)) << "FAILED" << reset()
                  << ": " << message << std::endl;
      }
    }

    void summarize(const test_results &results) {
      using namespace term;
      std::cout << format(sgr::bold) << results.passes << "/" << results.total
                << " tests passed";
      if(results.skips)
        std::cout << " (" << results.skips << " skipped)";
      std::cout << reset() << std::endl;

      for(auto &i : results.failures) {
        std::cout << "  " << i.test.full_name() << " "
                  << format(sgr::bold, fg(color::red)) << "FAILED" << reset()
                  << ": " << i.message << std::endl;
      }
    }
  private:
    bool verbose_;
  };
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

  term::colors_enabled = args.count("color");
  bool verbose = args.count("verbose");

  return run_tests(all_suites, single_run_logger(verbose));
}

#endif
