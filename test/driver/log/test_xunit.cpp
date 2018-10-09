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
        "<testsuites failures=\"0\" skipped=\"0\" tests=\"3\" "
                    "time=\"0.300000\">\n"
        "  <testsuite failures=\"0\" name=\"suite &gt; subsuite\" "
                     "skipped=\"0\" tests=\"1\" time=\"0.100000\">\n"
        "    <test name=\"test\" time=\"0.100000\">\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </test>\n"
        "  </testsuite>\n"
        "  <testsuite failures=\"0\" name=\"suite\" skipped=\"0\" tests=\"1\" "
                    "time=\"0.100000\">\n"
        "    <test name=\"test\" time=\"0.100000\">\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </test>\n"
        "  </testsuite>\n"
        "  <testsuite failures=\"0\" name=\"second suite\" skipped=\"0\" "
                     "tests=\"1\" time=\"0.100000\">\n"
        "    <test name=\"test\" time=\"0.100000\">\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </test>\n"
        "  </testsuite>\n"
        "</testsuites>\n"
      ));
    });

    _.test("failing run", [](logger_factory &f) {
      failing_run(f.logger);
      expect(f.ss->str(), equal_to(
        XML
        "<testsuites failures=\"1\" skipped=\"1\" tests=\"3\" "
                    "time=\"0.200000\">\n"
        "  <testsuite failures=\"0\" name=\"suite &gt; subsuite\" "
                     "skipped=\"1\" tests=\"1\" time=\"0.000000\">\n"
        "    <test name=\"test\">\n"
        "      <skipped message=\"message&#10;more\"/>\n"
        "    </test>\n"
        "  </testsuite>\n"
        "  <testsuite failures=\"0\" name=\"suite\" skipped=\"0\" tests=\"1\" "
                     "time=\"0.100000\">\n"
        "    <test name=\"test\" time=\"0.100000\">\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </test>\n"
        "  </testsuite>\n"
        "  <testsuite failures=\"1\" name=\"second suite\" skipped=\"0\" "
                     "tests=\"1\" time=\"0.100000\">\n"
        "    <test name=\"test\" time=\"0.100000\">\n"
        "      <failure message=\"error&#10;more\"/>\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </test>\n"
        "  </testsuite>\n"
        "</testsuites>\n"
      ));
    });

    _.test("failing file run", [](logger_factory &f) {
      failing_file_run(f.logger);
      expect(f.ss->str(), equal_to(
        XML
        "<testsuites failures=\"1\" skipped=\"1\" tests=\"4\" "
                    "time=\"0.200000\">\n"
        "  <testsuite failures=\"1\" name=\"file `test_file`\" tests=\"1\" "
                     "time=\"0\">\n"
        "    <test name=\"&lt;file&gt;\" time=\"0\">\n"
        "      <failure message=\"error&#10;more\"/>\n"
        "    </test>\n"
        "  </testsuite>\n"
        "  <testsuite failures=\"0\" name=\"second suite\" skipped=\"0\" "
                     "tests=\"1\" time=\"0.100000\">\n"
        "    <test name=\"test\" time=\"0.100000\">\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </test>\n"
        "  </testsuite>\n"
        "</testsuites>\n"
      ));
    });

    _.test("failing test and file run", [](logger_factory &f) {
      failing_test_and_file_run(f.logger);
      expect(f.ss->str(), equal_to(
        XML
        "<testsuites failures=\"2\" skipped=\"1\" tests=\"4\" "
                    "time=\"0.200000\">\n"
        "  <testsuite failures=\"1\" name=\"file `test_file`\" tests=\"1\" "
                     "time=\"0\">\n"
        "    <test name=\"&lt;file&gt;\" time=\"0\">\n"
        "      <failure message=\"error&#10;more\"/>\n"
        "    </test>\n"
        "  </testsuite>\n"
        "  <testsuite failures=\"1\" name=\"second suite\" skipped=\"0\" "
                     "tests=\"1\" time=\"0.100000\">\n"
        "    <test name=\"test\" time=\"0.100000\">\n"
        "      <failure message=\"error&#10;more\"/>\n"
        "      <system-out>\n"
        "        standard output\n"
        "      </system-out>\n"
        "      <system-err>\n"
        "        standard error\n"
        "      </system-err>\n"
        "    </test>\n"
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
