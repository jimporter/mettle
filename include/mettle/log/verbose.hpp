#ifndef INC_METTLE_LOG_VERBOSE_HPP
#define INC_METTLE_LOG_VERBOSE_HPP

#include <ostream>

#include "core.hpp"
#include "term.hpp"

namespace mettle {

namespace log {

  class verbose {
  public:
    verbose(std::ostream &out, unsigned int verbosity, bool show_terminal)
      : out(out), verbosity_(verbosity), show_terminal_(show_terminal),
        first_(true), base_indent_(0) {}

    void start_run() {
      first_ = true;
      if(verbosity_ == 1)
        out << std::string(base_indent_, ' ');
    }

    void end_run() {
      if(verbosity_ == 1)
        out << std::endl;
    }

    void start_suite(const std::vector<std::string> &suites) {
      using namespace term;
      if(verbosity_ >= 2) {
        if(!first_)
          out << std::endl;
        first_ = false;

        const std::string indent((suites.size() - 1) * 2 + base_indent_, ' ');
        out << indent << format(sgr::bold) << suites.back() << reset()
            << std::endl;
      }
    }

    void end_suite(const std::vector<std::string> &) {}

    void start_test(const test_name &test) {
      if(verbosity_ >= 2) {
        const std::string indent(test.suites.size() * 2 + base_indent_, ' ');
        out << indent << test.test << " " << std::flush;
      }
    }

    void passed_test(const test_name &test, const test_output &output) {
      using namespace term;
      if(verbosity_ == 0) {
        return;
      }
      else if(verbosity_ == 1) {
        out << format(sgr::bold, fg(color::green)) << "." << reset()
            << std::flush;
      }
      else {
        out << format(sgr::bold, fg(color::green)) << "PASSED" << reset()
            << std::endl;
        log_output(test.suites.size(), output);
      }
    }

    void failed_test(const test_name &test, const std::string &message,
                     const test_output &output) {
      using namespace term;
      if(verbosity_ == 0) {
        return;
      }
      else if(verbosity_ == 1) {
        out << format(sgr::bold, fg(color::red)) << "!" << reset()
            << std::flush;
      }
      else {
        out << format(sgr::bold, fg(color::red)) << "FAILED" << reset() << ": "
            << message << std::endl;
        log_output(test.suites.size(), output);
      }
    }

    void skipped_test(const test_name &) {
      using namespace term;
      if(verbosity_ == 0) {
        return;
      }
      else if(verbosity_ == 1) {
        out << format(sgr::bold, fg(color::blue)) << "_" << reset()
            << std::flush;
      }
      else {
        out << format(sgr::bold, fg(color::blue)) << "SKIPPED" << reset()
            << std::endl;
      }
    }

    unsigned int verbosity() const {
      return verbosity_;
    }

    void indent(size_t n) {
      base_indent_ = n;
    }

    std::ostream &out;
  private:
    void log_output(size_t depth, const test_output &output) {
      if(!show_terminal_)
        return;

      using namespace term;
      const std::string indent((depth + 1) * 2 + base_indent_, ' ');

      if(!output.stdout.empty()) {
        out << indent << format(fg(color::yellow)) << "stdout" << reset() << ":"
            << std::endl << output.stdout << std::endl;
      }
      if(!output.stderr.empty()) {
        out << indent << format(fg(color::yellow)) << "stderr" << reset() << ":"
            << std::endl << output.stderr << std::endl;
      }
    }

    unsigned int verbosity_;
    bool show_terminal_;
    bool first_;
    size_t base_indent_;
  };

}

} // namespace mettle

#endif
