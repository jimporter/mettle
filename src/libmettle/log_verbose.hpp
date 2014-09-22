#ifndef INC_METTLE_LIBMETTLE_LOG_VERBOSE_HPP
#define INC_METTLE_LIBMETTLE_LOG_VERBOSE_HPP

#include <mettle/log/core.hpp>
#include <mettle/log/indent.hpp>

namespace mettle {

namespace log {

  class verbose : public file_logger {
  public:
    verbose(indenting_ostream &out, size_t runs, bool show_terminal,
            bool show_time);

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
  private:
    void log_output(const test_output &output, bool extra_newline = false);

    indenting_ostream &out_;
    indenter indent_, run_indent_;
    size_t total_runs_, run_ = 0;
    bool first_ = true, show_terminal_, show_time_;
  };

}

} // namespace mettle

#endif
