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

  class verbose_logger {
  public:
    verbose_logger(unsigned int verbosity)
      : verbosity_(verbosity), first_(true), base_indent_(0) {}

    void start_run() {
      first_ = true;
      if(verbosity_ == 1)
        std::cout << std::string(base_indent_, ' ');
    }

    void end_run() {
      if(verbosity_ == 1)
        std::cout << std::endl;
    }

    void start_suite(const std::vector<std::string> &suites) {
      using namespace term;
      if(verbosity_ >= 2) {
        if(!first_)
          std::cout << std::endl;
        first_ = false;

        const std::string indent((suites.size() - 1) * 2 + base_indent_, ' ');
        std::cout << indent << format(sgr::bold) << suites.back()
                  << reset() << std::endl;
      }
    }

    void end_suite(const std::vector<std::string> &) {}

    void start_test(const test_name &test) {
      if(verbosity_ >= 2) {
        const std::string indent(test.suites.size() * 2 + base_indent_, ' ');
        std::cout << indent << test.test << " " << std::flush;
      }
    }

    void passed_test(const test_name &) {
      using namespace term;
      if(verbosity_ == 0) {
        return;
      }
      else if(verbosity_ == 1) {
        std::cout << format(sgr::bold, fg(color::green)) << "."
                  << reset() << std::flush;
      }
      else {
        std::cout << format(sgr::bold, fg(color::green)) << "PASSED" << reset()
                  << std::endl;
      }
    }

    void skipped_test(const test_name &) {
      using namespace term;
      if(verbosity_ == 0) {
        return;
      }
      else if(verbosity_ == 1) {
        std::cout << format(sgr::bold, fg(color::blue)) << "_"
                  << reset() << std::flush;
      }
      else {
        std::cout << format(sgr::bold, fg(color::blue)) << "SKIPPED" << reset()
                  << std::endl;
      }
    }

    void failed_test(const test_name &, const std::string &message) {
      using namespace term;
      if(verbosity_ == 0) {
        return;
      }
      else if(verbosity_ == 1) {
        std::cout << format(sgr::bold, fg(color::red)) << "!"
                  << reset() << std::flush;
      }
      else {
        std::cout << format(sgr::bold, fg(color::red)) << "FAILED" << reset()
                  << ": " << message << std::endl;
      }
    }

    unsigned int verbosity() const {
      return verbosity_;
    }

    void indent(size_t n) {
      base_indent_ = n;
    }
  private:
    unsigned int verbosity_;
    bool first_;
    size_t base_indent_;
  };

  class single_run_logger : public test_logger {
  public:
    single_run_logger(verbose_logger vlog)
      : vlog_(vlog), total_(0), passes_(0), skips_(0) {}

    void start_run() {
      vlog_.start_run();
    }

    void end_run() {
      vlog_.end_run();
    }

    void start_suite(const std::vector<std::string> &suites) {
      vlog_.start_suite(suites);
    }

    void end_suite(const std::vector<std::string> &suites) {
      vlog_.end_suite(suites);
    }

    void start_test(const test_name &test) {
      total_++;
      vlog_.start_test(test);
    }

    void passed_test(const test_name &test) {
      passes_++;
      vlog_.passed_test(test);
    }

    void skipped_test(const test_name &test) {
      skips_++;
      vlog_.skipped_test(test);
    }

    void failed_test(const test_name &test, const std::string &message) {
      failures_.push_back({test, message});
      vlog_.failed_test(test, message);
    }

    void summarize() {
      using namespace term;

      if(vlog_.verbosity())
        std::cout << std::endl;

      std::cout << format(sgr::bold) << passes_ << "/" << total_
                << " tests passed";
      if(skips_)
        std::cout << " (" << skips_ << " skipped)";
      std::cout << reset() << std::endl;

      for(auto &i : failures_) {
        std::cout << "  " << i.test.full_name() << " "
                  << format(sgr::bold, fg(color::red)) << "FAILED" << reset()
                  << ": " << i.message << std::endl;
      }
    }

    size_t failures() const {
      return failures_.size();
    }
  private:
    struct failure {
      test_name test;
      std::string message;
    };

    verbose_logger vlog_;
    size_t total_, passes_, skips_;
    std::vector<const failure> failures_;
  };

  class multi_run_logger : public test_logger {
  public:
    multi_run_logger(verbose_logger vlog)
      : vlog_(vlog), total_(0), skips_(0), runs_(0) {
      if(vlog_.verbosity() == 2)
        vlog_.indent(2);
    }

    void start_run() {
      using namespace term;
      runs_++;
      total_ = skips_ = 0;

      if(vlog_.verbosity() == 2) {
        if(runs_ > 1)
          std::cout << std::endl;
        std::cout << format(sgr::bold) << "Test run" << reset() << " "
                  << format(sgr::bold, fg(color::yellow)) << "[#" << runs_
                  << "]" << reset() << std::endl << std::endl;
      }
      vlog_.start_run();
    }

    void end_run() {
      vlog_.end_run();
    }

    void start_suite(const std::vector<std::string> &suites) {
      vlog_.start_suite(suites);
    }

    void end_suite(const std::vector<std::string> &suites) {
      vlog_.end_suite(suites);
    }

    void start_test(const test_name &test) {
      total_++;
      vlog_.start_test(test);
    }

    void passed_test(const test_name &test) {
      vlog_.passed_test(test);
    }

    void skipped_test(const test_name &test) {
      skips_++;
      vlog_.skipped_test(test);
    }

    void failed_test(const test_name &test, const std::string &message) {
      failures_[test].push_back({runs_, message});
      vlog_.failed_test(test, message);
    }

    void summarize() {
      using namespace term;
      size_t passes = total_ - skips_ - failures_.size();

      if(vlog_.verbosity())
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
    struct failure {
      size_t run;
      std::string message;
    };

    verbose_logger vlog_;
    size_t total_, skips_, runs_;
    std::map<test_name, std::vector<const failure>> failures_;
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
    ("verbose", opts::value<unsigned int>()->implicit_value(1),
     "show verbose output")
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
  verbose_logger vlog(
    args.count("verbose") ? args["verbose"].as<unsigned int>() : 0
  );

  if(args.count("runs")) {
    size_t runs = args["runs"].as<size_t>();
    if(runs == 0) {
      std::cout << "no test runs, exiting" << std::endl;
      return 1;
    }

    multi_run_logger logger(vlog);
    for(size_t i = 0; i < runs; i++)
      run_tests(all_suites, logger);
    logger.summarize();

    return logger.failures();
  }
  else {
    single_run_logger logger(vlog);
    run_tests(all_suites, logger);
    logger.summarize();

    return logger.failures();
  }
}

#endif
