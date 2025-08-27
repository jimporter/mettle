#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/log/xunit.hpp>

#include "log_runs.hpp"

#define XML "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"

struct logger_factory {
  logger_factory(std::size_t runs)
    : ss(new std::ostringstream()),
      logger(std::unique_ptr<std::ostream>(ss), runs) {}

  std::ostringstream *ss;
  log::xunit logger;
};

using namespace std::literals::chrono_literals;

suite<> test_verbose("xunit logger", [](auto &_) {
  subsuite<logger_factory>(_, "single run", bind_factory(1), [](auto &_) {
    _.test("passing run", [](logger_factory &f) {
      passing_run(f.logger);
      expect(f.ss->str(), equal_to(
        XML
        "<testsuites failures=\"0\" skipped=\"0\" tests=\"4\" "
                    "time=\"0.400000\">\n"
        "  <testsuite failures=\"0\" name=\"suite &gt; subsuite\" "
                     "skipped=\"0\" tests=\"1\" time=\"0.100000\">\n"
        "    <testcase file=\"file.cpp\" line=\"30\" name=\"test 3\" "
                      "time=\"0.100000\">\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </testcase>\n"
        "  </testsuite>\n"
        "  <testsuite failures=\"0\" name=\"suite\" skipped=\"0\" tests=\"2\" "
                     "time=\"0.200000\">\n"
        "    <testcase file=\"file.cpp\" line=\"10\" name=\"test 1\" "
                      "time=\"0.100000\"/>\n"
        "    <testcase file=\"file.cpp\" line=\"20\" name=\"test 2\" "
                      "time=\"0.100000\"/>\n"
        "  </testsuite>\n"
        "  <testsuite failures=\"0\" name=\"second suite\" skipped=\"0\" "
                     "tests=\"1\" time=\"0.100000\">\n"
        "    <testcase file=\"file.cpp\" line=\"40\" name=\"test 4\" "
                      "time=\"0.100000\">\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </testcase>\n"
        "  </testsuite>\n"
        "</testsuites>\n"
      ));
    });

    _.test("failing run", [](logger_factory &f) {
      failing_run(f.logger);
      expect(f.ss->str(), equal_to(
        XML
        "<testsuites failures=\"2\" skipped=\"1\" tests=\"4\" "
                    "time=\"0.300000\">\n"
        "  <testsuite failures=\"0\" name=\"suite &gt; subsuite\" "
                     "skipped=\"1\" tests=\"1\" time=\"0.000000\">\n"
        "    <testcase file=\"file.cpp\" line=\"30\" name=\"test 3\">\n"
        "      <skipped message=\"message&#10;more\"/>\n"
        "    </testcase>\n"
        "  </testsuite>\n"
        "  <testsuite failures=\"1\" name=\"suite\" skipped=\"0\" tests=\"2\" "
                     "time=\"0.200000\">\n"
        "    <testcase file=\"file.cpp\" line=\"10\" name=\"test 1\" "
                      "time=\"0.100000\">\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </testcase>\n"
        "    <testcase file=\"file.cpp\" line=\"20\" name=\"test 2\" "
                      "time=\"0.100000\">\n"
        "      <failure message=\"error\"/>\n"
        "    </testcase>\n"
        "  </testsuite>\n"
        "  <testsuite failures=\"1\" name=\"second suite\" skipped=\"0\" "
                     "tests=\"1\" time=\"0.100000\">\n"
        "    <testcase file=\"file.cpp\" line=\"40\" name=\"test 4\" "
                      "time=\"0.100000\">\n"
        "      <failure message=\"error&#10;more\"/>\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </testcase>\n"
        "  </testsuite>\n"
        "</testsuites>\n"
      ));
    });

    _.test("failing file run", [](logger_factory &f) {
      failing_file_run(f.logger);
      expect(f.ss->str(), equal_to(
        XML
        "<testsuites failures=\"1\" skipped=\"1\" tests=\"5\" "
                    "time=\"0.300000\">\n"
        "  <testsuite failures=\"1\" name=\"file `file.cpp`\" tests=\"1\" "
                     "time=\"0\">\n"
        "    <testcase name=\"&lt;file&gt;\" time=\"0\">\n"
        "      <failure message=\"error&#10;more\"/>\n"
        "    </testcase>\n"
        "  </testsuite>\n"
        "  <testsuite failures=\"0\" name=\"second suite\" skipped=\"0\" "
                     "tests=\"1\" time=\"0.100000\">\n"
        "    <testcase file=\"file.cpp\" line=\"40\" name=\"test 4\" "
                      "time=\"0.100000\">\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </testcase>\n"
        "  </testsuite>\n"
        "</testsuites>\n"
      ));
    });

    _.test("failing test and file run", [](logger_factory &f) {
      failing_test_and_file_run(f.logger);
      expect(f.ss->str(), equal_to(
        XML
        "<testsuites failures=\"3\" skipped=\"1\" tests=\"5\" "
                    "time=\"0.300000\">\n"
        "  <testsuite failures=\"1\" name=\"file `file.cpp`\" tests=\"1\" "
                     "time=\"0\">\n"
        "    <testcase name=\"&lt;file&gt;\" time=\"0\">\n"
        "      <failure message=\"error&#10;more\"/>\n"
        "    </testcase>\n"
        "  </testsuite>\n"
        "  <testsuite failures=\"1\" name=\"second suite\" skipped=\"0\" "
                     "tests=\"1\" time=\"0.100000\">\n"
        "    <testcase file=\"file.cpp\" line=\"40\" name=\"test 4\" "
                      "time=\"0.100000\">\n"
        "      <failure message=\"error&#10;more\"/>\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </testcase>\n"
        "  </testsuite>\n"
        "</testsuites>\n"
      ));
    });
  });

  _.test("multiple runs", []() {
    expect([]() { log::xunit("file.xml", 2); }, thrown<std::domain_error>(
      "xunit logger may only be used with --runs=1"
    ));
  });
});
