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

  class multi_run : public file_logger {
  public:
    multi_run(verbose vlog) : vlog_(vlog) {
      if(vlog_.verbosity() == 2)
        vlog_.indent(2);
    }

    void started_run() {
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
      vlog_.started_run();
    }

    void ended_run() {
      vlog_.ended_run();
    }

    void started_suite(const std::vector<std::string> &suites) {
      vlog_.started_suite(suites);
    }

    void ended_suite(const std::vector<std::string> &suites) {
      vlog_.ended_suite(suites);
    }

    void started_test(const test_name &test) {
      total_++;
      vlog_.started_test(test);
    }

    void passed_test(const test_name &test, const test_output &output) {
      vlog_.passed_test(test, output);
    }

    void failed_test(const test_name &test, const std::string &message,
                     const test_output &output) {
      failures_[test].push_back({runs_, message});
      vlog_.failed_test(test, message, output);
    }

    void skipped_test(const test_name &test, const std::string &message) {
      skips_++;
      vlog_.skipped_test(test, message);
    }

    void started_file(const std::string &) {}
    void ended_file(const std::string &) {}

    void failed_file(const std::string &file, const std::string &message) {
      // XXX: Distinguish between files executed multiple times per run?
      failed_files_[file].push_back({runs_, message});
      vlog_.failed_file(file, message);
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
      if(!failed_files_.empty()) {
        std::string s = failed_files_.size() > 1 ? "s" : "";
        vlog_.out << " [" << failed_files_.size() << " file" << s << " "
                  << format(fg(color::red)) << "FAILED"
                  << format(fg(color::normal)) << "]";
      }
      vlog_.out << reset() << std::endl;

      for(const auto &i : failures_)
        summarize_failure(i.first.full_name(), i.second);
      for(const auto &i : failed_files_)
        summarize_failure("`" + i.first + "`", i.second);
    }

    bool good() const {
      return failures_.empty();
    }
  private:
    struct failure {
      size_t run;
      std::string message;
    };

    void summarize_failure(const std::string &where,
                           const std::vector<const failure> &failures) {
      using namespace term;
      int run_width = std::ceil(std::log10(runs_));
      format fail_count_fmt(
        sgr::bold, fg(failures.size() == runs_ ? color::red : color::yellow)
      );
      vlog_.out << "  " << where << " " << format(sgr::bold, fg(color::red))
                << "FAILED" << reset() << " " << fail_count_fmt << "["
                << failures.size() << "/" << runs_ << "]" << reset() << ":"
                << std::endl;

      for(const auto &i : failures) {
        vlog_.out << "    " << i.message << " "
                  << format(sgr::bold, fg(color::yellow)) << "[#"
                  << std::setw(run_width) << i.run << "]" << reset()
                  << std::endl;
      }
    }

    verbose vlog_;
    size_t total_ = 0, skips_ = 0, runs_ = 0;
    std::map<test_name, std::vector<const failure>> failures_;
    std::map<std::string, std::vector<const failure>> failed_files_;
  };

}

} // namespace mettle

#endif
