#ifndef INC_METTLE_LOG_SINGLE_RUN_HPP
#define INC_METTLE_LOG_SINGLE_RUN_HPP

#include <ostream>

#include "core.hpp"
#include "term.hpp"
#include "verbose.hpp"

namespace mettle {

namespace log {

  class single_run : public test_logger {
  public:
    single_run(verbose vlog) : vlog_(vlog), total_(0), passes_(0), skips_(0) {}

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

    void passed_test(const test_name &test, test_output &output) {
      passes_++;
      vlog_.passed_test(test, output);
    }

    void failed_test(const test_name &test, const std::string &message,
                     test_output &output) {
      failures_.push_back({test, message});
      vlog_.failed_test(test, message, output);
    }

    void skipped_test(const test_name &test) {
      skips_++;
      vlog_.skipped_test(test);
    }

    void summarize() {
      using namespace term;

      if(vlog_.verbosity())
        vlog_.out << std::endl;

      vlog_.out << format(sgr::bold) << passes_ << "/" << total_
                << " tests passed";
      if(skips_)
        vlog_.out << " (" << skips_ << " skipped)";
      vlog_.out << reset() << std::endl;

      for(const auto &i : failures_) {
        vlog_.out << "  " << i.test.full_name() << " "
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

    verbose vlog_;
    size_t total_, passes_, skips_;
    std::vector<const failure> failures_;
  };

}

} // namespace mettle

#endif
