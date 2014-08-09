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
    single_run(verbose vlog) : vlog_(vlog) {}

    void started_run() {
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
      passes_++;
      vlog_.passed_test(test, output);
    }

    void failed_test(const test_name &test, const std::string &message,
                     const test_output &output) {
      failures_.push_back({test, message});
      vlog_.failed_test(test, message, output);
    }

    void skipped_test(const test_name &test) {
      skips_++;
      vlog_.skipped_test(test);
    }

    void failed_file(const std::string &file, const std::string &message) {
      failed_files_.push_back({file, message});
      vlog_.failed_file(file, message);
    }

    void summarize() {
      using namespace term;

      if(vlog_.verbosity())
        vlog_.out << std::endl;

      vlog_.out << format(sgr::bold) << passes_ << "/" << total_
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
        summarize_failure(i.test.full_name(), i.message);
      for(const auto &i : failed_files_)
        summarize_failure("`" + i.file + "`", i.message);
    }

    bool good() const {
      return failures_.empty();
    }
  private:
    struct test_failure {
      test_name test;
      std::string message;
    };

    struct file_failure {
      std::string file;
      std::string message;
    };

    void summarize_failure(const std::string &where,
                           const std::string &message) {
      using namespace term;
      vlog_.out << "  " << where << " " << format(sgr::bold, fg(color::red))
                << "FAILED" << reset() << ": " << message << std::endl;
    }

    verbose vlog_;
    size_t total_ = 0, passes_ = 0, skips_ = 0;
    std::vector<const test_failure> failures_;
    std::vector<const file_failure> failed_files_;
  };

}

} // namespace mettle

#endif
