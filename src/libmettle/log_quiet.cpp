#include "log_quiet.hpp"

#include <mettle/log/term.hpp>

namespace mettle {

namespace log {

  quiet::quiet(indenting_ostream &out)
    : out_(out) {}

  void quiet::started_run() {}

  void quiet::ended_run() {
    out_ << std::endl;
  }

  void quiet::started_suite(const std::vector<std::string> &) {}
  void quiet::ended_suite(const std::vector<std::string> &) {}

  void quiet::started_test(const test_name &) {}

  void quiet::passed_test(const test_name &, const test_output &,
                          test_duration) {
    using namespace term;
    out_ << format(sgr::bold, fg(color::green)) << "." << reset()
         << std::flush;
  }

  void quiet::failed_test(const test_name &, const std::string &,
                          const test_output &, test_duration) {
    using namespace term;
    out_ << format(sgr::bold, fg(color::red)) << "!" << reset() << std::flush;
  }

  void quiet::skipped_test(const test_name &, const std::string &) {
    using namespace term;
    out_ << format(sgr::bold, fg(color::blue)) << "_" << reset()
         << std::flush;
  }

  void quiet::started_file(const std::string &) {}
  void quiet::ended_file(const std::string &) {}

  void quiet::failed_file(const std::string &, const std::string &) {
    using namespace term;
    out_ << format(sgr::bold, fg(color::red)) << "X" << reset() << std::flush;
  }

}

} // namespace mettle
