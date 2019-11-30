#include <mettle/driver/log/counter.hpp>

#include <iomanip>

#include <mettle/driver/log/term.hpp>

namespace mettle::log {

  counter::counter(indenting_ostream &out)
    : out_(out) {}

  void counter::started_run() {
    tests_ = 0;
    passes_ = 0;
    skips_ = 0;
    failures_ = 0;
    print_counter();
  }

  void counter::ended_run() {
    out_ << std::endl;
  }

  void counter::started_suite(const std::vector<std::string> &) {}
  void counter::ended_suite(const std::vector<std::string> &) {}

  void counter::started_test(const test_name &) {}

  void counter::passed_test(const test_name &, const test_output &,
                            test_duration) {
    tests_++;
    passes_++;
    print_counter();
  }

  void counter::failed_test(const test_name &, const std::string &,
                            const test_output &, test_duration) {
    tests_++;
    failures_++;
    print_counter();
  }

  void counter::skipped_test(const test_name &, const std::string &) {
    tests_++;
    skips_++;
    print_counter();
  }

  void counter::started_file(const std::string &) {}
  void counter::ended_file(const std::string &) {}

  void counter::failed_file(const std::string &, const std::string &) {
    tests_++;
    failures_++;
    print_counter();
  }

  void counter::print_counter() {
    using namespace term;
    format all(sgr::bold);
    format passed(sgr::bold, fg(color::green));
    format skipped(sgr::bold, fg(color::blue));
    format failed(sgr::bold, fg(color::red));

    out_ << "\r[ " << all     << std::setw(3) << tests_    << reset()
         << " | "  << passed  << std::setw(3) << passes_   << reset()
         << " | "  << skipped << std::setw(3) << skips_    << reset()
         << " | "  << failed  << std::setw(3) << failures_ << reset()
         << " ]"   << std::flush;
  }

} // namespace mettle::log
