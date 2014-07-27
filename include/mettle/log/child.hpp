#ifndef INC_METTLE_LOG_CHILD_HPP
#define INC_METTLE_LOG_CHILD_HPP

#include <ostream>
#include <iomanip>

#include <bencode.hpp>

#include "term.hpp"
#include "core.hpp"

namespace mettle {

namespace log {

  inline void encode(std::ostream &os, const test_name &test) {
    bencode::encode_dict(os,
      "id", test.id,
      "suites", test.suites,
      "test", test.test
    );
  }

  inline void encode(std::ostream &os, const test_output &output) {
    bencode::encode_dict(os,
      "stderr", output.stderr,
      "stdout", output.stdout
    );
  }

  class child : public test_logger {
  public:
    child(std::ostream &out) : out(out) {}

    void start_run() {
      bencode::encode_dict(out, "event", "start_run");
      out.flush();
    }
    void end_run() {
      bencode::encode_dict(out, "event", "end_run");
      out.flush();
    }

    void start_suite(const std::vector<std::string> &suites) {
      bencode::encode_dict(out,
        "event", "start_suite",
        "suites", suites
      );
      out.flush();
    }
    void end_suite(const std::vector<std::string> &suites) {
      bencode::encode_dict(out,
        "event", "end_suite",
        "suites", suites
      );
      out.flush();
    }

    void start_test(const test_name &test) {
      bencode::encode_dict(out,
        "event", "start_test",
        "test", test
      );
      out.flush();
    }

    void passed_test(const test_name &test, const test_output &output) {
      bencode::encode_dict(out,
        "event", "passed_test",
        "output", output,
        "test", test
      );
      out.flush();
    }
    void failed_test(const test_name &test, const std::string &message,
                     const test_output &output) {
      bencode::encode_dict(out,
        "event", "failed_test",
        "message", message,
        "output", output,
        "test", test
      );
      out.flush();
    }
    void skipped_test(const test_name &test) {
      bencode::encode_dict(out,
        "event", "skipped_test",
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
