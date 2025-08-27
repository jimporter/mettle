#include <mettle/driver/log/brief.hpp>

#include <mettle/driver/log/term.hpp>

namespace mettle::log {

  brief::brief(indenting_ostream &out)
    : out_(out) {}

  void brief::started_run() {}

  void brief::ended_run() {
    out_ << std::endl;
  }

  void brief::started_suite(const std::vector<std::string> &) {}
  void brief::ended_suite(const std::vector<std::string> &) {}

  void brief::started_test(const test_name &) {}

  void brief::passed_test(const test_name &, const test_output &,
                          test_duration) {
    using namespace term;
    out_ << format(sgr::bold, sgr::inverse, fg(color::green)) << "." << reset()
         << std::flush;
  }

  void brief::failed_test(const test_name &, const std::string &,
                          const test_output &, test_duration) {
    using namespace term;
    out_ << format(sgr::bold, sgr::inverse, fg(color::red)) << "!" << reset()
         << std::flush;
  }

  void brief::skipped_test(const test_name &, const std::string &) {
    using namespace term;
    out_ << format(sgr::bold, sgr::inverse, fg(color::blue)) << "_" << reset()
         << std::flush;
  }

  void brief::started_file(const test_file &) {}
  void brief::ended_file(const test_file &) {}

  void brief::failed_file(const test_file &, const std::string &) {
    using namespace term;
    out_ << format(sgr::bold, sgr::inverse, fg(color::red)) << "X" << reset()
         << std::flush;
  }

} // namespace mettle::log
