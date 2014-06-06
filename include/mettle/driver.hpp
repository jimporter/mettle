#ifndef INC_METTLE_DRIVER_HPP
#define INC_METTLE_DRIVER_HPP

#include <cmath>
#include <iomanip>
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

  class multi_run_logger : public test_logger {
  public:
    multi_run_logger(bool verbose)
      : verbose_(verbose), skips_(0), total_(0), runs_(0) {}

    void start_suite(const std::vector<std::string> &) {}
    void end_suite(const std::vector<std::string> &) {}

    void start_test(const test_name &) {}
    void passed_test(const test_name &) {
      using namespace term;
      if(verbose_) {
        std::cout << format(sgr::bold, fg(color::green)) << "."
                  << reset() << std::flush;
      }
    }

    void skipped_test(const test_name &) {
      using namespace term;
      if(verbose_) {
        std::cout << format(sgr::bold, fg(color::blue)) << "_"
                  << reset() << std::flush;
      }
    }

    void failed_test(const test_name &, const std::string &) {
      using namespace term;
      if(verbose_) {
        std::cout << format(sgr::bold, fg(color::red)) << "!"
                  << reset() << std::flush;
      }
    }

    void summarize(const test_results &results) {
      if(verbose_)
        std::cout << std::endl;

      runs_++;
      skips_ = results.skips;
      total_ = results.total;
      for(auto &i : results.failures)
        failures_[i.test].push_back({runs_, i.message});
    }

    void summarize_all() {
      using namespace term;
      size_t passes = total_ - skips_ - failures_.size();

      if(verbose_)
        std::cout << std::endl;

      std::cout << format(sgr::bold) << passes << "/" << total_
                << " tests passed";
      if(skips_)
        std::cout << " (" << skips_ << " skipped)";
      std::cout << reset() << std::endl;

      int run_width = std::ceil(std::log10(runs_));
      for(auto &i : failures_) {
        format fail_count_fmt(
          sgr::bold, fg(i.second.size() == runs_ ? color::red : color::yellow)
        );
        std::cout << "  " << i.first.full_name() << " "
                  << format(sgr::bold, fg(color::red)) << "FAILED" << reset()
                  << " " << fail_count_fmt << "[" << i.second.size() << "/"
                  << runs_ << "]" << reset() << ":" << std::endl;

        for(auto &j : i.second) {
          std::cout << "    " << j.message << " "
                    << format(sgr::bold, fg(color::yellow)) << "["
                    << std::setw(run_width) << j.run << "]" << reset()
                    << std::endl;
        }
      }
    }

    size_t failures() const {
      return failures_.size();
    }
  private:
    struct failure_info {
      size_t run;
      std::string message;
    };

    bool verbose_;
    size_t skips_, total_, runs_;
    std::map<test_name, std::vector<failure_info>> failures_;
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
    ("runs", opts::value<size_t>(), "number of test runs")
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

  if(args.count("runs")) {
    size_t runs = args["runs"].as<size_t>();
    if(runs == 0) {
      std::cout << "no test runs, exiting" << std::endl;
      return 1;
    }
    multi_run_logger logger(verbose);
    for(size_t i = 0; i < runs; i++)
      run_tests(all_suites, logger);
    logger.summarize_all();

    return logger.failures();
  }
  else {
    return run_tests(all_suites, single_run_logger(verbose));
  }
}

#endif
