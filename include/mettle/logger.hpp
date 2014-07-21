#ifndef INC_METTLE_LOGGER_HPP
#define INC_METTLE_LOGGER_HPP

#include "runner.hpp"
#include "term.hpp"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>

namespace mettle {

class verbose_logger {
public:
  verbose_logger(std::ostream &out, unsigned int verbosity,
                 bool show_terminal)
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

  void passed_test(const test_name &test, test_output_log &log) {
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
      log_output(test.suites.size(), log);
    }
  }

  void failed_test(const test_name &test, const std::string &message,
                   test_output_log &log) {
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
      log_output(test.suites.size(), log);
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
  void log_output(size_t depth, test_output_log &log) {
    if(!show_terminal_)
      return;

    using namespace term;
    const std::string indent((depth + 1) * 2 + base_indent_, ' ');

    if(log.stdout.peek() != EOF) {
      out << indent << format(fg(color::yellow)) << "stdout" << reset() << ":"
          << std::endl << log.stdout.rdbuf() << std::endl;
    }
    if(log.stderr.peek() != EOF) {
      out << indent << format(fg(color::yellow)) << "stderr" << reset() << ":"
          << std::endl << log.stderr.rdbuf() << std::endl;
    }
  }

  unsigned int verbosity_;
  bool show_terminal_;
  bool first_;
  size_t base_indent_;
};

class single_run_logger : public test_logger {
public:
  single_run_logger(verbose_logger vlog)
    : vlog_(vlog), total_(0), passes_(0), skips_(0) {}

  void start_run() {
    vlog_.start_run();
  }

  void end_run() {
    vlog_.end_run();
  }

  void start_suite(const std::vector<std::string> &suites) {
    vlog_.start_suite(suites);
  }

  void end_suite(const std::vector<std::string> &suites) {
    vlog_.end_suite(suites);
  }

  void start_test(const test_name &test) {
    total_++;
    vlog_.start_test(test);
  }

  void passed_test(const test_name &test, test_output_log &log) {
    passes_++;
    vlog_.passed_test(test, log);
  }

  void failed_test(const test_name &test, const std::string &message,
                   test_output_log &log) {
    failures_.push_back({test, message});
    vlog_.failed_test(test, message, log);
  }

  void skipped_test(const test_name &test) {
    skips_++;
    vlog_.skipped_test(test);
  }

  void summarize() {
    using namespace term;

    if(vlog_.verbosity())
      vlog_.out << std::endl;

    vlog_.out << format(sgr::bold) << passes_ << "/" << total_
              << " tests passed";
    if(skips_)
      vlog_.out << " (" << skips_ << " skipped)";
    vlog_.out << reset() << std::endl;

    for(const auto &i : failures_) {
      vlog_.out << "  " << i.test.full_name() << " "
                << format(sgr::bold, fg(color::red)) << "FAILED" << reset()
                << ": " << i.message << std::endl;
    }
  }

  size_t failures() const {
    return failures_.size();
  }
private:
  struct failure {
    test_name test;
    std::string message;
  };

  verbose_logger vlog_;
  size_t total_, passes_, skips_;
  std::vector<const failure> failures_;
};

class multi_run_logger : public test_logger {
public:
  multi_run_logger(verbose_logger vlog)
    : vlog_(vlog), total_(0), skips_(0), runs_(0) {
    if(vlog_.verbosity() == 2)
      vlog_.indent(2);
  }

  void start_run() {
    using namespace term;
    runs_++;
    total_ = skips_ = 0;

    if(vlog_.verbosity() == 2) {
      if(runs_ > 1)
        vlog_.out << std::endl;
      vlog_.out << format(sgr::bold) << "Test run" << reset() << " "
                << format(sgr::bold, fg(color::yellow)) << "[#" << runs_
                << "]" << reset() << std::endl << std::endl;
    }
    vlog_.start_run();
  }

  void end_run() {
    vlog_.end_run();
  }

  void start_suite(const std::vector<std::string> &suites) {
    vlog_.start_suite(suites);
  }

  void end_suite(const std::vector<std::string> &suites) {
    vlog_.end_suite(suites);
  }

  void start_test(const test_name &test) {
    total_++;
    vlog_.start_test(test);
  }

  void passed_test(const test_name &test, test_output_log &log) {
    vlog_.passed_test(test, log);
  }

  void failed_test(const test_name &test, const std::string &message,
                   test_output_log &log) {
    failures_[test].push_back({runs_, message});
    vlog_.failed_test(test, message, log);
  }

  void skipped_test(const test_name &test) {
    skips_++;
    vlog_.skipped_test(test);
  }

  void summarize() {
    using namespace term;
    size_t passes = total_ - skips_ - failures_.size();

    if(vlog_.verbosity())
      vlog_.out << std::endl;

    vlog_.out << format(sgr::bold) << passes << "/" << total_
              << " tests passed";
    if(skips_)
      vlog_.out << " (" << skips_ << " skipped)";
    vlog_.out << reset() << std::endl;

    int run_width = std::ceil(std::log10(runs_));
    for(const auto &i : failures_) {
      format fail_count_fmt(
        sgr::bold, fg(i.second.size() == runs_ ? color::red : color::yellow)
      );
      vlog_.out << "  " << i.first.full_name() << " "
                << format(sgr::bold, fg(color::red)) << "FAILED" << reset()
                << " " << fail_count_fmt << "[" << i.second.size() << "/"
                << runs_ << "]" << reset() << ":" << std::endl;

      for(const auto &j : i.second) {
        vlog_.out << "    " << j.message << " "
                  << format(sgr::bold, fg(color::yellow)) << "["
                  << std::setw(run_width) << j.run << "]" << reset()
                  << std::endl;
      }
    }
  }

  size_t failures() const {
    return failures_.size();
  }
private:
  struct failure {
    size_t run;
    std::string message;
  };

  verbose_logger vlog_;
  size_t total_, skips_, runs_;
  std::map<test_name, std::vector<const failure>> failures_;
};

} // namespace mettle

#endif
