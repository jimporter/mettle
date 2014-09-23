#ifndef INC_METTLE_LOG_SUMMARY_HPP
#define INC_METTLE_LOG_SUMMARY_HPP

#include <chrono>
#include <iomanip>
#include <sstream>

#include "core.hpp"
#include "indent.hpp"
#include "term.hpp"

namespace mettle {

namespace log {

  class summary : public file_logger {
  public:
    summary(indenting_ostream &out, file_logger *log, bool show_time)
      : out_(out), log_(log), show_time_(show_time) {}

    void started_run() {
      if(log_) log_->started_run();

      if(show_time_ && runs_ == 0)
        start_time_ = std::chrono::steady_clock::now();

      runs_++;
      total_ = file_index_ = 0;
    }

    void ended_run() {
      if(log_) log_->ended_run();
    }

    void started_suite(const std::vector<std::string> &suites) {
      if(log_) log_->started_suite(suites);
    }

    void ended_suite(const std::vector<std::string> &suites) {
      if(log_) log_->ended_suite(suites);
    }

    void started_test(const test_name &test) {
      if(log_) log_->started_test(test);

      total_++;
    }

    void passed_test(const test_name &test, const test_output &output,
                     test_duration duration) {
      if(log_) log_->passed_test(test, output, duration);
    }

    void failed_test(const test_name &test, const std::string &message,
                     const test_output &output, test_duration duration) {
      if(log_) log_->failed_test(test, message, output, duration);

      failures_[test].push_back({runs_, message});
    }

    void skipped_test(const test_name &test, const std::string &message) {
      if(log_) log_->skipped_test(test, message);

      skips_.emplace(test, message);
    }

    void started_file(const std::string &file) {
      if(log_) log_->started_file(file);
    }

    void ended_file(const std::string &file) {
      if(log_) log_->ended_file(file);

      file_index_++;
    }

    void failed_file(const std::string &file, const std::string &message) {
      if(log_) log_->failed_file(file, message);

      failed_files_[{file_index_++, file}].push_back({runs_, message});
    }

    void summarize() const {
      assert(runs_ > 0 && "number of runs can't be zero");

      if(log_)
        out_ << std::endl;

      using namespace term;
      size_t passes = total_ - skips_.size() - failures_.size();

      out_ << format(sgr::bold) << passes << "/" << total_ << " tests passed";

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

    bool good() const {
      return failures_.empty();
    }
  private:
    struct file_info {
      size_t index;
      std::string name;

      bool operator <(const file_info &rhs) const {
        return index < rhs.index;
      }
    };

    struct failure {
      size_t run;
      std::string message;
    };

    void summarize_skip(const std::string &test,
                        const std::string &message) const {
      using namespace term;

      out_ << test << " " << format(sgr::bold, fg(color::blue)) << "SKIPPED"
           << reset() << std::endl;
      if(!message.empty()) {
        scoped_indent si(out_);
        out_ << message << std::endl;
      }
    }

    void summarize_failure(const std::string &where,
                           const std::vector<const failure> &failures) const {
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
      }
      else {
        int run_width = std::ceil(std::log10(runs_));
        for(const auto &i : failures) {
          out_ << format(sgr::bold, fg(color::yellow)) << "[#"
               << std::setw(run_width) << i.run << "]" << reset() << " ";
          scoped_indent si(out_, indent_style::visual, run_width + 4);
          out_ << i.message << std::endl;
        }
      }
    }

    indenting_ostream &out_;
    file_logger *log_;
    bool show_time_;
    std::chrono::steady_clock::time_point start_time_;
    size_t total_ = 0, runs_ = 0, file_index_ = 0;
    std::map<test_name, std::vector<const failure>> failures_;
    std::map<test_name, std::string> skips_;
    std::map<file_info, std::vector<const failure>> failed_files_;
  };

}

} // namespace mettle

#endif
