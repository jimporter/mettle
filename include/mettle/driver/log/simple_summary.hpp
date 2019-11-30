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

    void started_suite(const std::vector<std::string> &) override {}
    void ended_suite(const std::vector<std::string> &) override {}

    void started_test(const test_name &) override {
      total_++;
    }

    void passed_test(const test_name &, const test_output &,
                     test_duration) override {}

    void failed_test(const test_name &test, const std::string &message,
                     const test_output &, test_duration) override {
      failures_.push_back({test, message});
    }

    void skipped_test(const test_name &test,
                      const std::string &message) override {
      skips_.push_back({test, message});
    }

    void summarize() const {
      std::size_t passes = total_ - skips_.size() - failures_.size();

      out_ << passes << "/" << total_ << " tests passed";
      if(!skips_.empty())
        out_ << " (" << skips_.size() << " skipped)";
      out_ << std::endl;

      scoped_indent indent(out_);
      for(const auto &i : skips_)
        summarize_test(i, false);
      for(const auto &i : failures_)
        summarize_test(i, true);
    }

    bool good() const {
      return failures_.empty();
    }
  private:
    struct test_details {
      test_name test;
      std::string message;
    };

    void summarize_test(const test_details &details, bool failure) const {
      out_ << details.test.full_name() << " "
           << (failure ? "FAILED" : "SKIPPED") << std::endl;
      if(!details.message.empty()) {
        scoped_indent si(out_);
        out_ << details.message << std::endl;
      }
    }

    indenting_ostream &out_;
    std::size_t total_ = 0;
    std::vector<test_details> failures_, skips_;
  };

} // namespace mettle::log

#endif
