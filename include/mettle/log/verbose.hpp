#ifndef INC_METTLE_LOG_VERBOSE_HPP
#define INC_METTLE_LOG_VERBOSE_HPP

#include "core.hpp"
#include "indent.hpp"
#include "term.hpp"

namespace mettle {

namespace log {

  class verbose : public file_logger {
  public:
    verbose(indenting_ostream &out, size_t runs, bool show_terminal)
      : out_(out), indent_(out), run_indent_(out), total_runs_(runs),
        show_terminal_(show_terminal) {}

    void started_run() {
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

    void ended_run() {
      if(total_runs_ > 1)
        run_indent_--;
    }

    void started_suite(const std::vector<std::string> &suites) {
      using namespace term;

      if(!first_)
          out_ << std::endl;
      first_ = false;

      out_ << format(sgr::bold) << suites.back() << reset() << std::endl;
      indent_++;
    }

    void ended_suite(const std::vector<std::string> &) {
      indent_--;
    }

    void started_test(const test_name &test) {
      out_ << test.test << " " << std::flush;
    }

    void passed_test(const test_name &, const test_output &output) {
      using namespace term;
      out_ << format(sgr::bold, fg(color::green)) << "PASSED" << reset()
           << std::endl;

      scoped_indent si(out_);
      log_output(output);
    }

    void failed_test(const test_name &, const std::string &message,
                     const test_output &output) {
      using namespace term;
      out_ << format(sgr::bold, fg(color::red)) << "FAILED" << reset()
           << std::endl;

      scoped_indent si(out_);
      if(!message.empty())
        out_ << message << std::endl;
      log_output(output, !message.empty());
    }

    void skipped_test(const test_name &, const std::string &message) {
      using namespace term;
      out_ << format(sgr::bold, fg(color::blue)) << "SKIPPED" << reset()
           << std::endl;

      if(!message.empty()) {
        scoped_indent si(out_);
        out_ << message << std::endl;
      }
    }

    void started_file(const std::string &) {}

    void ended_file(const std::string &) {
      indent_.reset();
    }

    void failed_file(const std::string &file, const std::string &message) {
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
  private:
    void log_output(const test_output &output, bool extra_newline = false) {
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

    indenting_ostream &out_;
    indenter indent_, run_indent_;
    size_t total_runs_, run_ = 0;
    bool first_ = true, show_terminal_;
  };

}

} // namespace mettle

#endif
