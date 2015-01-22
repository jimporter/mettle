#include <mettle/driver/log/summary.hpp>

#include <cassert>
#include <cmath>

#include <mettle/driver/log/term.hpp>

namespace mettle {

namespace log {

  summary::summary(indenting_ostream &out, std::unique_ptr<file_logger> &&log,
                   bool show_time, bool show_terminal)
    : out_(out), log_(std::move(log)), show_time_(show_time),
      show_terminal_(show_terminal) {}

  void summary::started_run() {
    if(log_) log_->started_run();

    if(show_time_ && runs_ == 0)
      start_time_ = std::chrono::steady_clock::now();

    runs_++;
    total_ = file_index_ = 0;
  }

  void summary::ended_run() {
    if(log_) log_->ended_run();
  }

  void summary::started_suite(const std::vector<std::string> &suites) {
    if(log_) log_->started_suite(suites);
  }

  void summary::ended_suite(const std::vector<std::string> &suites) {
    if(log_) log_->ended_suite(suites);
  }

  void summary::started_test(const test_name &test) {
    if(log_) log_->started_test(test);

    total_++;
  }

  void summary::passed_test(const test_name &test, const test_output &output,
                            test_duration duration) {
    if(log_) log_->passed_test(test, output, duration);
  }

  void summary::failed_test(const test_name &test, const std::string &message,
                            const test_output &output, test_duration duration) {
    if(log_) log_->failed_test(test, message, output, duration);

    failures_[test].push_back({
      runs_, message, show_terminal_ ? output : log::test_output()
    });
  }

  void summary::skipped_test(const test_name &test,
                             const std::string &message) {
    if(log_) log_->skipped_test(test, message);

    skips_.emplace(test, message);
  }

  void summary::started_file(const std::string &file) {
    if(log_) log_->started_file(file);
  }

  void summary::ended_file(const std::string &file) {
    if(log_) log_->ended_file(file);

    file_index_++;
  }

  void summary::failed_file(const std::string &file,
                            const std::string &message) {
    if(log_) log_->failed_file(file, message);

    failed_files_[{file_index_++, file}].push_back({runs_, message, {}});
  }

  void summary::summarize() const {
    assert(runs_ > 0 && "number of runs can't be zero");

    if(log_)
      out_ << std::endl;

    using namespace term;
    std::size_t passes = total_ - skips_.size() - failures_.size();
    std::string test_str = total_ == 1 ? "test" : "tests";

    out_ << format(sgr::bold) << passes << "/" << total_ << " "
         << (passes == 1 && total_ == 1 ? "test" : "tests") << " passed";

    if(!skips_.empty())
      out_ << " (" << skips_.size() << " skipped)";

    if(!failed_files_.empty()) {
      std::string s = failed_files_.size() > 1 ? "s" : "";
      out_ << " [" << failed_files_.size() << " file" << s << " "
           << format(fg(color::red)) << "FAILED" << format(fg(color::normal))
           << "]";
    }

    if(show_time_) {
      using namespace std::chrono;
      auto elapsed = duration_cast<duration<float>>(
        steady_clock::now() - start_time_
      );
      std::ostringstream elapsed_str;
      elapsed_str << std::setprecision(4) << elapsed.count() << " s";
      out_ << " " << format(fg(color::black)) << "(took "
           << elapsed_str.str() << ")";
    }

    out_ << reset() << std::endl;

    scoped_indent indent(out_);
    // XXX: Interleave skips and failures in the appropriate order?
    for(const auto &i : skips_)
      summarize_skip(i.first.full_name(), i.second);
    for(const auto &i : failures_)
      summarize_failure(i.first.full_name(), i.second);
    for(const auto &i : failed_files_)
      summarize_failure("`" + i.first.name + "`", i.second);
  }

  void summary::summarize_skip(const std::string &test,
                               const std::string &message) const {
    using namespace term;

    out_ << test << " " << format(sgr::bold, fg(color::blue)) << "SKIPPED"
         << reset() << std::endl;
    if(!message.empty()) {
      scoped_indent si(out_);
      out_ << message << std::endl;
    }
  }

  void summary::summarize_failure(
    const std::string &where, const std::vector<failure> &failures
  ) const {
    using namespace term;

    out_ << where << " " << format(sgr::bold, fg(color::red)) << "FAILED"
         << reset();
    if(runs_ > 1) {
      format fail_count_fmt(
        sgr::bold, fg(failures.size() == runs_ ? color::red : color::yellow)
      );
      out_ << " " << fail_count_fmt << "[" << failures.size() << "/" << runs_
           << "]" << reset();
    }
    out_ << std::endl;

    scoped_indent si(out_);
    if(runs_ == 1) {
      auto &&message = failures[0].message;
      if(!message.empty())
        out_ << message << std::endl;
      log_output(failures[0].output, !message.empty());
    }
    else {
      int run_width = static_cast<int>( std::ceil(std::log10(runs_)) );
      for(const auto &i : failures) {
        out_ << format(sgr::bold, fg(color::yellow)) << "[#"
             << std::setw(run_width) << i.run << "]" << reset() << " ";
        scoped_indent sii(out_, indent_style::visual, run_width + 4);
        out_ << i.message << std::endl;
        log_output(i.output, true);
      }
    }
  }

  void summary::log_output(const test_output &output,
                           bool extra_newline) const {
    if(!show_terminal_)
      return;

    using namespace term;

    bool has_output = !output.stdout_log.empty() || !output.stderr_log.empty();
    if(extra_newline && has_output)
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

}

} // namespace mettle
