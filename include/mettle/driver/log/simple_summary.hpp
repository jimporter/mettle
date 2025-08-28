#ifndef INC_METTLE_DRIVER_LOG_SIMPLE_SUMMARY_HPP
#define INC_METTLE_DRIVER_LOG_SIMPLE_SUMMARY_HPP

#include <cstdint>
#include <vector>

#include "core.hpp"
#include "indent.hpp"

namespace mettle::log {

  class simple_summary : public test_logger {
  public:
    simple_summary(indenting_ostream &out) : out_(out) {}

    void started_run() override {}
    void ended_run() override {}

    void started_suite(const std::vector<suite_name> &) override {}
    void ended_suite(const std::vector<suite_name> &) override {}

    void started_test(const test_name &) override {
      total_++;
    }

    void passed_test(const test_name &, const test_output &,
                     test_duration) override {}

    void failed_test(const test_name &test, const test_failure &failure,
                     const test_output &, test_duration) override {
      unpasses_.push_back({test, true, failure.message});
      failures_++;
    }

    void skipped_test(const test_name &test,
                      const std::string &message) override {
      unpasses_.push_back({test, false, message});
      skips_++;
    }

    void summarize() const {
      std::size_t passes = total_ - unpasses_.size();

      out_ << passes << "/" << total_ << " tests passed";
      if(skips_)
        out_ << " (" << skips_ << " skipped)";
      out_ << std::endl;

      scoped_indent indent(out_);
      for(const auto &i : unpasses_)
        summarize_test(i);
    }

    bool good() const {
      return failures_ == 0;
    }
  private:
    struct test_details {
      test_name test;
      bool failure;
      std::string message;
    };

    void summarize_test(const test_details &details) const {
      out_ << details.test.full_name() << " "
           << (details.failure ? "FAILED" : "SKIPPED") << std::endl;
      if(!details.message.empty()) {
        scoped_indent si(out_);
        out_ << details.message << std::endl;
      }
    }

    indenting_ostream &out_;
    std::size_t total_ = 0, skips_ = 0, failures_ = 0;
    std::vector<test_details> unpasses_;
  };

} // namespace mettle::log

#endif
