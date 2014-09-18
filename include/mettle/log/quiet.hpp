#ifndef INC_METTLE_LOG_QUIET_HPP
#define INC_METTLE_LOG_QUIET_HPP

#include "core.hpp"
#include "indent.hpp"
#include "term.hpp"

namespace mettle {

namespace log {

  class quiet : public file_logger {
  public:
    quiet(indenting_ostream &out)
      : out_(out) {}

    void started_run() {}

    void ended_run() {
      out_ << std::endl;
    }

    void started_suite(const std::vector<std::string> &) {}
    void ended_suite(const std::vector<std::string> &) {}

    void started_test(const test_name &) {}

    void passed_test(const test_name &, const test_output &) {
      using namespace term;
      out_ << format(sgr::bold, fg(color::green)) << "." << reset()
           << std::flush;
    }

    void failed_test(const test_name &, const std::string &,
                     const test_output &) {
      using namespace term;
      out_ << format(sgr::bold, fg(color::red)) << "!" << reset() << std::flush;
    }

    void skipped_test(const test_name &, const std::string &) {
      using namespace term;
      out_ << format(sgr::bold, fg(color::blue)) << "_" << reset()
           << std::flush;
    }

    void started_file(const std::string &) {}
    void ended_file(const std::string &) {}

    void failed_file(const std::string &, const std::string &) {
      using namespace term;
      out_ << format(sgr::bold, fg(color::red)) << "X" << reset() << std::flush;
    }

  private:
    indenting_ostream& out_;
  };

}

} // namespace mettle

#endif
