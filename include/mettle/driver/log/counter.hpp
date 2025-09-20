#ifndef INC_METTLE_DRIVER_LOG_COUNTER_HPP
#define INC_METTLE_DRIVER_LOG_COUNTER_HPP

#include <cstdint>

#include "core.hpp"
#include "indent.hpp"
#include "../detail/export.hpp"

// Ignore warnings from MSVC about DLL interfaces.
#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(push)
#  pragma warning(disable:4251)
#endif

namespace mettle::log {

  class METTLE_PUBLIC counter : public file_logger {
  public:
    counter(indenting_ostream &out);

    void started_run() override;
    void ended_run() override;

    void started_suite(const std::vector<suite_name> &suites) override;
    void ended_suite(const std::vector<suite_name> &suites) override;

    void started_test(const test_name &test) override;
    void passed_test(const test_name &test, const test_output &output,
                     test_duration duration) override;
    void failed_test(const test_name &test, const std::string &message,
                     const test_output &output,
                     test_duration duration) override;
    void skipped_test(const test_name &test,
                      const std::string &message) override;

    void started_file(const test_file &file) override;
    void ended_file(const test_file &file) override;

    void failed_file(const test_file &file,
                     const std::string &message) override;
  private:
    void print_counter();

    indenting_ostream &out_;
    std::size_t tests_ = 0, passes_ = 0, skips_ = 0, failures_ = 0;
  };

} // namespace mettle::log

#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(pop)
#endif

#endif
