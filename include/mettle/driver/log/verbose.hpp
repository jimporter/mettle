#ifndef INC_METTLE_DRIVER_LOG_VERBOSE_HPP
#define INC_METTLE_DRIVER_LOG_VERBOSE_HPP

#include <cstdint>

#include "core.hpp"
#include "indent.hpp"
#include "../detail/export.hpp"

#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable:4251)
#endif

namespace mettle {

namespace log {

  class METTLE_PUBLIC verbose : public file_logger {
  public:
    verbose(indenting_ostream &out, std::size_t runs, bool show_time,
            bool show_terminal);

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
    void log_output(const test_output &output, bool extra_newline) const;

    indenting_ostream &out_;
    indenter indent_, run_indent_;
    std::size_t total_runs_, run_ = 0;
    bool first_ = true, show_time_, show_terminal_;
  };

}

} // namespace mettle

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

#endif
