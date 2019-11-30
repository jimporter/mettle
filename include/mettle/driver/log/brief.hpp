#ifndef INC_METTLE_DRIVER_LOG_BRIEF_HPP
#define INC_METTLE_DRIVER_LOG_BRIEF_HPP

#include "core.hpp"
#include "indent.hpp"
#include "../detail/export.hpp"

namespace mettle::log {

  class METTLE_PUBLIC brief : public file_logger {
  public:
    brief(indenting_ostream &out);

    void started_run() override;
    void ended_run() override;

    void started_suite(const std::vector<std::string> &suites) override;
    void ended_suite(const std::vector<std::string> &suites) override;

    void started_test(const test_name &test) override;
    void passed_test(const test_name &test, const test_output &output,
                     test_duration duration) override;
    void failed_test(const test_name &test, const std::string &message,
                     const test_output &output,
                     test_duration duration) override;
    void skipped_test(const test_name &test,
                      const std::string &message) override;

    void started_file(const std::string &file) override;
    void ended_file(const std::string &file) override;

    void failed_file(const std::string &file,
                     const std::string &message) override;
  private:
    indenting_ostream &out_;
  };

} // namespace mettle::log

#endif
