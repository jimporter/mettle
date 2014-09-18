#ifndef INC_METTLE_LOG_SUMMARY_HPP
#define INC_METTLE_LOG_SUMMARY_HPP

#include <iomanip>

#include "core.hpp"
#include "indent.hpp"
#include "term.hpp"

namespace mettle {

namespace log {

  class summary : public file_logger {
  public:
    summary(indenting_ostream &out, file_logger *log) : out_(out), log_(log) {}

    void started_run() {
      if(log_) log_->started_run();

      runs_++;
      total_ = skips_ = 0;
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

    void passed_test(const test_name &test, const test_output &output) {
      if(log_) log_->passed_test(test, output);
    }

    void failed_test(const test_name &test, const std::string &message,
                     const test_output &output) {
      if(log_) log_->failed_test(test, message, output);

      failures_[test].push_back({runs_, message});
    }

    void skipped_test(const test_name &test, const std::string &message) {
      if(log_) log_->skipped_test(test, message);

      skips_++;
    }

    void started_file(const std::string &file) {
      if(log_) log_->started_file(file);
    }

    void ended_file(const std::string &file) {
      if(log_) log_->ended_file(file);
    }

    void failed_file(const std::string &file, const std::string &message) {
      if(log_) log_->failed_file(file, message);

      // XXX: Distinguish between files executed multiple times per run?
      failed_files_[file].push_back({runs_, message});
    }

    void summarize() {
      assert(runs_ > 0 && "number of runs can't be zero");

      if(log_)
        out_ << std::endl;

      using namespace term;
      size_t passes = total_ - skips_ - failures_.size();

      out_ << format(sgr::bold) << passes << "/" << total_ << " tests passed";
      if(skips_)
        out_ << " (" << skips_ << " skipped)";
      if(!failed_files_.empty()) {
        std::string s = failed_files_.size() > 1 ? "s" : "";
        out_ << " [" << failed_files_.size() << " file" << s << " "
             << format(fg(color::red)) << "FAILED" << format(fg(color::normal))
             << "]";
      }
      out_ << reset() << std::endl;

      scoped_indent indent(out_);
      for(const auto &i : failures_)
        summarize_failure(i.first.full_name(), i.second);
      for(const auto &i : failed_files_)
        summarize_failure("`" + i.first + "`", i.second);
    }

    bool good() const {
      return failures_.empty();
    }
  private:
    struct failure {
      size_t run;
      std::string message;
    };

    void summarize_failure(const std::string &where,
                           const std::vector<const failure> &failures) {
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
    size_t total_ = 0, skips_ = 0, runs_ = 0;
    std::map<test_name, std::vector<const failure>> failures_;
    std::map<std::string, std::vector<const failure>> failed_files_;
  };

}

} // namespace mettle

#endif
