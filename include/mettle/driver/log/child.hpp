#ifndef INC_METTLE_DRIVER_LOG_CHILD_HPP
#define INC_METTLE_DRIVER_LOG_CHILD_HPP

#include <cassert>
#include <ostream>

#include <bencode.hpp>

#include "core.hpp"

namespace mettle {

inline void encode(std::ostream &os, const test_name &test) {
  bencode::encode_dict(os,
    "id", test.id,
    "suites", test.suites,
    "test", test.test
  );
}

namespace log {

  inline void encode(std::ostream &os, const test_output &output) {
    bencode::encode_dict(os,
      "stderr_log", output.stderr_log,
      "stdout_log", output.stdout_log
    );
  }

  class child : public test_logger {
  public:
    child(std::ostream &out) : out(out) {}

    void started_run() {
      bencode::encode_dict(out, "event", "started_run");
      out.flush();
    }
    void ended_run() {
      bencode::encode_dict(out, "event", "ended_run");
      out.flush();
    }

    void started_suite(const std::vector<std::string> &suites) {
      bencode::encode_dict(out,
        "event", "started_suite",
        "suites", suites
      );
      out.flush();
    }
    void ended_suite(const std::vector<std::string> &suites) {
      bencode::encode_dict(out,
        "event", "ended_suite",
        "suites", suites
      );
      out.flush();
    }

    void started_test(const test_name &test) {
      bencode::encode_dict(out,
        "event", "started_test",
        "test", test
      );
      out.flush();
    }

    void passed_test(const test_name &test, const test_output &output,
                     test_duration duration) {
      bencode::encode_dict(out,
        "duration", duration.count(),
        "event", "passed_test",
        "output", output,
        "test", test
      );
      out.flush();
    }
    void failed_test(const test_name &test, const std::string &message,
                     const test_output &output, test_duration duration) {
      bencode::encode_dict(out,
        "duration", duration.count(),
        "event", "failed_test",
        "message", message,
        "output", output,
        "test", test
      );
      out.flush();
    }
    void skipped_test(const test_name &test, const std::string &message) {
      bencode::encode_dict(out,
        "event", "skipped_test",
        "message", message,
        "test", test
      );
      out.flush();
    }
  private:
    std::ostream &out;
  };

}

} // namespace mettle

#endif
