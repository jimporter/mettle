#ifndef INC_METTLE_DRIVER_LOG_SUMMARY_HPP
#define INC_METTLE_DRIVER_LOG_SUMMARY_HPP

#include <cstdint>
#include <chrono>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>

#include "core.hpp"
#include "indent.hpp"
#include "../detail/export.hpp"

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable:4251)
#endif

namespace mettle {

namespace log {

  class METTLE_PUBLIC summary : public file_logger {
  public:
    summary(indenting_ostream &out, std::unique_ptr<file_logger> &&log,
            bool show_time, bool show_terminal);

    void started_run();
    void ended_run();

    void started_suite(const std::vector<std::string> &suites);
    void ended_suite(const std::vector<std::string> &suites);

    void started_test(const test_name &test);
    void passed_test(const test_name &test, const test_output &output,
                     test_duration duration);
    void failed_test(const test_name &test, const std::string &message,
                     const test_output &output, test_duration duration);
    void skipped_test(const test_name &test, const std::string &message);

    void started_file(const std::string &file);
    void ended_file(const std::string &file);
    void failed_file(const std::string &file, const std::string &message);

    void summarize() const;

    bool good() const {
      return failures_.empty();
    }
  private:
    struct file_info {
      std::size_t index;
      std::string name;

      bool operator <(const file_info &rhs) const {
        return index < rhs.index;
      }
    };

    struct failure {
      std::size_t run;
      std::string message;
      log::test_output output;
    };

    void summarize_skip(const std::string &test,
                        const std::string &message) const;
    void summarize_failure(const std::string &where,
                           const std::vector<failure> &failures) const;
    void log_output(const test_output &output, bool extra_newline) const;

    indenting_ostream &out_;
    std::unique_ptr<file_logger> log_;
    bool show_time_, show_terminal_;
    std::chrono::steady_clock::time_point start_time_;

    std::size_t total_ = 0, runs_ = 0, file_index_ = 0;
    std::map<test_name, std::vector<failure>> failures_;
    std::map<test_name, std::string> skips_;
    std::map<file_info, std::vector<failure>> failed_files_;
  };

}

} // namespace mettle

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#endif
