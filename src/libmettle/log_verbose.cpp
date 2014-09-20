#include "log_verbose.hpp"

#include <cassert>

#include <mettle/log/term.hpp>

namespace mettle {

namespace log {

  verbose::verbose(indenting_ostream &out, size_t runs, bool show_terminal)
    : out_(out), indent_(out), run_indent_(out), total_runs_(runs),
      show_terminal_(show_terminal) {}

  void verbose::started_run() {
    assert(run_ < total_runs_ && "tests were run too many times");
    using namespace term;

    first_ = true;

    if(total_runs_ > 1) {
      if(++run_ > 1)
        out_ << std::endl;

      out_ << format(sgr::bold) << "Test run" << reset() << " "
           << format(sgr::bold, fg(color::yellow)) << "[#" << run_ << "/"
           << total_runs_ << "]" << reset() << std::endl << std::endl;

      run_indent_++;
    }
  }

  void verbose::ended_run() {
    if(total_runs_ > 1)
      run_indent_--;
  }

  void verbose::started_suite(const std::vector<std::string> &suites) {
    using namespace term;

    if(!first_)
      out_ << std::endl;
    first_ = false;

    out_ << format(sgr::bold) << suites.back() << reset() << std::endl;
    indent_++;
  }

  void verbose::ended_suite(const std::vector<std::string> &) {
    indent_--;
  }

  void verbose::started_test(const test_name &test) {
    out_ << test.test << " " << std::flush;
  }

  void verbose::passed_test(const test_name &, const test_output &output) {
    using namespace term;
    out_ << format(sgr::bold, fg(color::green)) << "PASSED" << reset()
         << std::endl;

    scoped_indent si(out_);
    log_output(output);
  }

  void verbose::failed_test(const test_name &, const std::string &message,
                            const test_output &output) {
    using namespace term;
    out_ << format(sgr::bold, fg(color::red)) << "FAILED" << reset()
         << std::endl;

    scoped_indent si(out_);
    if(!message.empty())
      out_ << message << std::endl;
    log_output(output, !message.empty());
  }

  void verbose::skipped_test(const test_name &, const std::string &message) {
    using namespace term;
    out_ << format(sgr::bold, fg(color::blue)) << "SKIPPED" << reset()
         << std::endl;

    if(!message.empty()) {
      scoped_indent si(out_);
      out_ << message << std::endl;
    }
  }

  void verbose::started_file(const std::string &) {}

  void verbose::ended_file(const std::string &) {
    indent_.reset();
  }

  void verbose::failed_file(const std::string &file,
                            const std::string &message) {
    indent_.reset();

    using namespace term;

    if(!first_)
      out_ << std::endl;
    first_ = false;

    out_ << "`" << file << "` " << format(sgr::bold, fg(color::red))
         << "FAILED" << reset() << std::endl;
    scoped_indent si(out_);
    out_ << message << std::endl;
  }

  void verbose::log_output(const test_output &output, bool extra_newline) {
    if(!show_terminal_)
      return;

    using namespace term;

    if(extra_newline && (!output.stdout.empty() || !output.stderr.empty()))
      out_ << std::endl;

    if(!output.stdout.empty()) {
      out_ << format(fg(color::yellow), sgr::underline) << "stdout" << reset()
           << ":" << std::endl << output.stdout << std::endl;
    }
    if(!output.stderr.empty()) {
      out_ << format(fg(color::yellow), sgr::underline) << "stderr" << reset()
           << ":" << std::endl << output.stderr << std::endl;
    }
  }

}

} // namespace mettle
