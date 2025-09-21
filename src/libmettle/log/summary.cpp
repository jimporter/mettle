#include <mettle/driver/log/summary.hpp>

#include <cassert>
#include <cmath>

#include <boost/io/ios_state.hpp>

#include <mettle/driver/log/format.hpp>
#include <mettle/driver/log/term.hpp>

namespace mettle::log {

  namespace {
    template<typename T>
    std::string to_term_string(T &&t, bool term_enabled) {
      std::ostringstream ss;
      term::enable(ss, term_enabled);
      ss << t;
      return ss.str();
    }
  }

  summary::summary(indenting_ostream &out, std::unique_ptr<file_logger> &&log,
                   bool show_time, bool show_terminal)
    : out_(out), log_(std::move(log)), show_time_(show_time),
      show_terminal_(show_terminal) {}

  void summary::started_run() {
    if(log_) log_->started_run();

    if(show_time_ && runs_ == 0)
      start_time_ = std::chrono::steady_clock::now();

    runs_++;
    total_ = 0;
  }

  void summary::ended_run() {
    if(log_) log_->ended_run();
  }

  void summary::started_suite(const std::vector<suite_name> &suites) {
    if(log_) log_->started_suite(suites);
  }

  void summary::ended_suite(const std::vector<suite_name> &suites) {
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

  void summary::failed_test(const test_name &test, const test_failure &failure,
                            const test_output &output, test_duration duration) {
    if(log_) log_->failed_test(test, failure, output, duration);

    bool term_enabled = term::is_enabled(out_);

    add_unpass(test.id, to_term_string(test, term_enabled), fail)
      .failures.emplace_back(runs_, to_term_string(failure, term_enabled),
                             show_terminal_ ? output : log::test_output{});
  }

  void summary::skipped_test(const test_name &test,
                             const std::string &message) {
    if(log_) log_->skipped_test(test, message);

    bool term_enabled = term::is_enabled(out_);

    add_unpass(test.id, to_term_string(test, term_enabled), skip).skip_message =
      message;
  }

  void summary::started_file(const test_file &file) {
    if(log_) log_->started_file(file);
  }

  void summary::ended_file(const test_file &file) {
    if(log_) log_->ended_file(file);
  }

  void summary::failed_file(const test_file &file,
                            const std::string &message) {
    if(log_) log_->failed_file(file, message);

    // Max out the local bits of the UID so that it sorts *after* regular
    // file-and-test UIDs.
    test_uid sortid = detail::max_local_bits(file.id);

    bool term_enabled = term::is_enabled(out_);
    std::ostringstream ss;
    term::enable(ss, term_enabled);
    ss << "`" << term::link(file.name) << file.name << term::link() << "`";

    add_unpass(sortid, ss.str(), file_fail).failures.emplace_back(
      runs_, message, log::test_output{}
    );
  }

  void summary::summarize() const {
    assert(runs_ > 0 && "number of runs can't be zero");

    if(log_)
      out_ << std::endl;

    using namespace term;
    std::size_t passes = total_ - unpass_counts_[skip] - unpass_counts_[fail];
    std::string test_str = total_ == 1 ? "test" : "tests";

    out_ << format(sgr::bold) << passes << "/" << total_ << " "
         << (passes == 1 && total_ == 1 ? "test" : "tests") << " passed";

    if(unpass_counts_[skip])
      out_ << " (" << unpass_counts_[skip] << " skipped)";

    if(unpass_counts_[file_fail]) {
      std::string s = unpass_counts_[file_fail] > 1 ? "s" : "";
      out_ << " [" << unpass_counts_[file_fail] << " file" << s << " "
           << format(fg(color::red)) << "FAILED" << format(fg(color::normal))
           << "]";
    }

    if(show_time_) {
      using namespace std::chrono;
      auto elapsed = duration_cast<duration<float>>(
        steady_clock::now() - start_time_
      );

      boost::io::ios_all_saver ias(out_);
      out_ << " " << format(fg(color::black)) << "(took " << std::fixed
           << std::setprecision(4) << elapsed.count() << " s)";
    }

    out_ << reset() << std::endl;

    scoped_indent indent(out_);
    for(const auto &i : unpasses_) {
      if(i.second.type == skip)
        summarize_skip(i.second.name, i.second.skip_message);
      else
        summarize_failure(i.second.name, i.second.failures);
    }
  }

  bool summary::good() const {
    return unpass_counts_[fail] == 0 && unpass_counts_[file_fail] == 0;
  }

  summary::unpass &
  summary::add_unpass(test_uid id, std::string name, unpass_type type) {
    assert(type >= 0 && type < 3 && "invalid type value");
    auto [it, inserted] = unpasses_.emplace(id, unpass{std::move(name), type});
    if(inserted)
      unpass_counts_[type]++;
    return it->second;
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
      log_output(failures[0].output, true);
    } else {
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
