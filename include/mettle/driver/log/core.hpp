#ifndef INC_METTLE_DRIVER_LOG_CORE_HPP
#define INC_METTLE_DRIVER_LOG_CORE_HPP

#include <chrono>
#include <string>
#include <vector>

#include "../test_name.hpp"
#include "../detail/export.hpp"

namespace mettle {

namespace log {

  struct test_output {
    std::string stdout_log, stderr_log;
  };

  using test_duration = std::chrono::milliseconds;

  class METTLE_PUBLIC test_logger {
  public:
    virtual ~test_logger() {}

    virtual void
    started_run() = 0;
    virtual void
    ended_run() = 0;

    virtual void
    started_suite(const std::vector<std::string> &suites) = 0;
    virtual void
    ended_suite(const std::vector<std::string> &suites) = 0;

    virtual void
    started_test(const test_name &test) = 0;
    virtual void
    passed_test(const test_name &test, const test_output &output,
                test_duration duration) = 0;
    virtual void
    failed_test(const test_name &test, const std::string &message,
                const test_output &output, test_duration duration) = 0;
    virtual void
    skipped_test(const test_name &test, const std::string &message) = 0;
  };

  class METTLE_PUBLIC file_logger : public test_logger {
  public:
    virtual ~file_logger() {}

    virtual void
    started_file(const std::string &file) = 0;
    virtual void
    ended_file(const std::string &file) = 0;
    virtual void
    failed_file(const std::string &file, const std::string &message) = 0;
  };

}

} // namespace mettle

#endif
