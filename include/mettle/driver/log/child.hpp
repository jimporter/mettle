#ifndef INC_METTLE_DRIVER_LOG_CHILD_HPP
#define INC_METTLE_DRIVER_LOG_CHILD_HPP

#include <cassert>
#include <ostream>

#include <bencode.hpp>

#include "core.hpp"

namespace mettle::log {

  class child : public test_logger {
  public:
    child(std::ostream &out) : out(out) {}

    void started_run() override {
      bencode::encode(out, bencode::dict_view{
        {"event", "started_run"}
      });
      out.flush();
    }
    void ended_run() override {
      bencode::encode(out, bencode::dict_view{
        {"event", "ended_run"}
      });
      out.flush();
    }

    void started_suite(const std::vector<std::string> &suites) override {
      bencode::encode(out, bencode::dict_view{
        {"event", "started_suite"},
        {"suites", wrap_suites(suites)}
      });
      out.flush();
    }
    void ended_suite(const std::vector<std::string> &suites) override {
      bencode::encode(out, bencode::dict_view{
        {"event", "ended_suite"},
        {"suites", wrap_suites(suites)}
      });
      out.flush();
    }

    void started_test(const test_name &test) override {
      bencode::encode(out, bencode::dict_view{
        {"event", "started_test"},
        {"test", wrap_test(test)}
      });
      out.flush();
    }

    void passed_test(const test_name &test, const test_output &output,
                     test_duration duration) override {
      bencode::encode(out, bencode::dict_view{
        {"event", "passed_test"},
        {"test", wrap_test(test)},
        {"duration", duration.count()},
        {"output", wrap_output(output)}
      });
      out.flush();
    }

    void failed_test(const test_name &test, const std::string &message,
                     const test_output &output,
                     test_duration duration) override {
      bencode::encode(out, bencode::dict_view{
        {"event", "failed_test"},
        {"test", wrap_test(test)},
        {"duration", duration.count()},
        {"message", message},
        {"output", wrap_output(output)}
      });
      out.flush();
    }

    void skipped_test(const test_name &test,
                      const std::string &message) override {
      bencode::encode(out, bencode::dict_view{
        {"event", "skipped_test"},
        {"test", wrap_test(test)},
        {"message", message}
      });
      out.flush();
    }
  private:
    bencode::dict_view wrap_test(const test_name &test) {
      return bencode::dict_view{
        {"id", bencode::integer(test.id)},
        {"suites", wrap_suites(test.suites)},
        {"test", test.name},
        {"file", test.file},
        {"line", test.line},
      };
    }

    bencode::dict_view wrap_output(const test_output &output) {
      return bencode::dict_view{
        {"stdout_log", output.stdout_log},
        {"stderr_log", output.stderr_log}
      };
    }

    bencode::list_view wrap_suites(const std::vector<std::string> &suites) {
      bencode::list_view result;
      for(auto &&i : suites)
        result.push_back(i);
      return result;
    }

    std::ostream &out;
  };

} // namespace mettle::log

#endif
