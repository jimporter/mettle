#ifndef INC_METTLE_LOG_MULTI_RUN_HPP
#define INC_METTLE_LOG_MULTI_RUN_HPP

#include <cmath>
#include <iomanip>
#include <map>

#include "core.hpp"
#include "term.hpp"
#include "verbose.hpp"

namespace mettle {

namespace log {

  class multi_run : public test_logger {
  public:
    multi_run(verbose vlog) : vlog_(vlog), total_(0), skips_(0), runs_(0) {
      if(vlog_.verbosity() == 2)
        vlog_.indent(2);
    }

    void start_run() {
      using namespace term;
      runs_++;
      total_ = skips_ = 0;

      if(vlog_.verbosity() == 2) {
        if(runs_ > 1)
          vlog_.out << std::endl;
        vlog_.out << format(sgr::bold) << "Test run" << reset() << " "
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

    void passed_test(const test_name &test, test_output &output) {
      vlog_.passed_test(test, output);
    }

    void failed_test(const test_name &test, const std::string &message,
                     test_output &output) {
      failures_[test].push_back({runs_, message});
      vlog_.failed_test(test, message, output);
    }

    void skipped_test(const test_name &test) {
      skips_++;
      vlog_.skipped_test(test);
    }

    void summarize() {
      using namespace term;
      size_t passes = total_ - skips_ - failures_.size();

      if(vlog_.verbosity())
        vlog_.out << std::endl;

      vlog_.out << format(sgr::bold) << passes << "/" << total_
                << " tests passed";
      if(skips_)
        vlog_.out << " (" << skips_ << " skipped)";
      vlog_.out << reset() << std::endl;

      int run_width = std::ceil(std::log10(runs_));
      for(const auto &i : failures_) {
        format fail_count_fmt(
          sgr::bold, fg(i.second.size() == runs_ ? color::red : color::yellow)
        );
        vlog_.out << "  " << i.first.full_name() << " "
                  << format(sgr::bold, fg(color::red)) << "FAILED" << reset()
                  << " " << fail_count_fmt << "[" << i.second.size() << "/"
                  << runs_ << "]" << reset() << ":" << std::endl;

        for(const auto &j : i.second) {
          vlog_.out << "    " << j.message << " "
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

    verbose vlog_;
    size_t total_, skips_, runs_;
    std::map<test_name, std::vector<const failure>> failures_;
  };

}

} // namespace mettle

#endif
