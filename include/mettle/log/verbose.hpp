#ifndef INC_METTLE_LOG_VERBOSE_HPP
#define INC_METTLE_LOG_VERBOSE_HPP

#include <ostream>

#include "core.hpp"
#include "indent.hpp"
#include "term.hpp"

namespace mettle {

namespace log {

  class verbose {
  public:
    verbose(std::ostream &os, unsigned int verbosity, bool show_terminal)
      : out(os), indent_(out), verbosity_(verbosity),
        show_terminal_(show_terminal) {}

    void started_run() {
      first_ = true;
    }

    void ended_run() {
      if(verbosity_ == 1)
        out << std::endl;
    }

    void started_suite(const std::vector<std::string> &suites) {
      using namespace term;
      if(verbosity_ >= 2) {
        if(!first_)
          out << std::endl;
        first_ = false;

        out << format(sgr::bold) << suites.back() << reset() << std::endl;
        indent_++;
      }
    }

    void ended_suite(const std::vector<std::string> &) {
      if(verbosity_ >= 2)
        indent_--;
    }

    void started_test(const test_name &test) {
      if(verbosity_ >= 2)
        out << test.test << " " << std::flush;
    }

    void passed_test(const test_name &, const test_output &output) {
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

        scoped_indent si(out);
        log_output(output);
      }
    }

    void failed_test(const test_name &, const std::string &message,
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
        out << format(sgr::bold, fg(color::red)) << "FAILED" << reset()
            << std::endl;

        scoped_indent si(out);
        if(!message.empty())
          out << message << std::endl;
        log_output(output, !message.empty());
      }
    }

    void skipped_test(const test_name &, const std::string &message) {
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

        if(!message.empty()) {
          scoped_indent si(out);
          out << message << std::endl;
        }
      }
    }

    void started_file(const std::string &) {}

    void ended_file(const std::string &) {
      indent_.reset();
    }

    void failed_file(const std::string &file, const std::string &message) {
      indent_.reset();

      using namespace term;
      if(verbosity_ == 0) {
        return;
      }
      else if(verbosity_ == 1) {
        out << format(sgr::bold, fg(color::red)) << "X" << reset()
            << std::flush;
      }
      else {
        if(!first_)
          out << std::endl;
        first_ = false;
        out << "`" << file << "` " << format(sgr::bold, fg(color::red))
            << "FAILED" << reset() << std::endl;
        scoped_indent si(out);
        out << message << std::endl;
      }
    }

    unsigned int verbosity() const {
      return verbosity_;
    }

    indenting_ostream out;
  private:
    void log_output(const test_output &output, bool extra_newline = false) {
      if(!show_terminal_)
        return;

      using namespace term;

      if(extra_newline && (!output.stdout.empty() || !output.stderr.empty()))
        out << std::endl;

      if(!output.stdout.empty()) {
        out << format(fg(color::yellow), sgr::underline) << "stdout" << reset()
            << ":" << std::endl << output.stdout << std::endl;
      }
      if(!output.stderr.empty()) {
        out << format(fg(color::yellow), sgr::underline) << "stderr" << reset()
            << ":" << std::endl << output.stderr << std::endl;
      }
    }

    indenter indent_;
    unsigned int verbosity_;
    bool show_terminal_;
    bool first_ = true;
  };

}

} // namespace mettle

#endif
