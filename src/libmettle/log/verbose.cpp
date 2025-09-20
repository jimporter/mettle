#include <mettle/driver/log/verbose.hpp>

#include <cassert>

#include <mettle/driver/log/term.hpp>

namespace mettle::log {

  verbose::verbose(indenting_ostream &out, std::size_t runs, bool show_time,
                   bool show_terminal)
    : out_(out), indent_(out), run_indent_(out), total_runs_(runs),
      show_time_(show_time), show_terminal_(show_terminal) {}

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

  void verbose::started_suite(const std::vector<suite_name> &suites) {
    using namespace term;

    if(!first_)
      out_ << std::endl;
    first_ = false;

    out_ << format(sgr::bold) << suites.back().name << reset() << std::endl;
    indent_++;
  }

  void verbose::ended_suite(const std::vector<suite_name> &) {
    indent_--;
  }

  void verbose::started_test(const test_name &test) {
    out_ << test.name << " " << std::flush;
  }

  void verbose::passed_test(const test_name &, const test_output &output,
                            test_duration duration) {
    using namespace term;
    out_ << format(sgr::bold, fg(color::green)) << "PASSED" << reset();
    summarize_output(output);
    log_time(duration);
    out_ << std::endl;

    scoped_indent si(out_);
    log_output(output, false);
  }

  void verbose::failed_test(const test_name &, const test_failure &failure,
                            const test_output &output, test_duration duration) {
    using namespace term;
    out_ << format(sgr::bold, fg(color::red)) << "FAILED" << reset();
    summarize_output(output);
    log_time(duration);
    out_ << std::endl;

    scoped_indent si(out_);
    if(!failure.message.empty())
      out_ << failure.message << std::endl;
    log_output(output, !failure.message.empty());
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

  void verbose::started_file(const test_file &) {}

  void verbose::ended_file(const test_file &) {
    indent_.reset();
  }

  void verbose::failed_file(const test_file &file,
                            const std::string &message) {
    using namespace term;

    indent_.reset();

    if(!first_)
      out_ << std::endl;
    first_ = false;

    out_ << "`" << file.name << "` " << format(sgr::bold, fg(color::red))
         << "FAILED" << reset() << std::endl;
    scoped_indent si(out_);
    out_ << message << std::endl;
  }

  void verbose::log_time(test_duration duration) const {
    using namespace term;
    if(show_time_) {
      out_ << " " << format(sgr::bold, fg(color::black)) << "("
           << duration.count() << " ms)" << reset();
    }
  }

  void verbose::summarize_output(const test_output &output) const {
    if(show_terminal_ || output.empty())
      return;

    using namespace term;
    out_ << " " << format(fg(color::yellow)) << "[...]" << reset();
  }

  void verbose::log_output(const test_output &output,
                           bool extra_newline) const {
    if(!show_terminal_ || output.empty())
      return;

    using namespace term;

    if(extra_newline)
      out_ << std::endl;

    if(!output.stdout_log.empty()) {
      out_ << format(fg(color::yellow), sgr::underline) << "stdout" << reset()
           << ":" << std::endl << output.stdout_log << std::endl;
    }
    if(!output.stderr_log.empty()) {
      out_ << format(fg(color::yellow), sgr::underline) << "stderr" << reset()
           << ":" << std::endl << output.stderr_log << std::endl;
    }
  }

} // namespace mettle::log
