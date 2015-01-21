#ifndef INC_METTLE_DRIVER_LOG_BRIEF_HPP
#define INC_METTLE_DRIVER_LOG_BRIEF_HPP

#include "core.hpp"
#include "indent.hpp"
#include "../detail/export.hpp"

namespace mettle {

namespace log {

  class METTLE_PUBLIC brief : public file_logger {
  public:
    brief(indenting_ostream &out);

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
    indenting_ostream &out_;
  };

}

} // namespace mettle

#endif
