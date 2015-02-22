#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/log/simple_summary.hpp>
#include <mettle/driver/log/indent.hpp>

#include "log_runs.hpp"

struct logger_factory {
  logger_factory()
    : is(ss), logger(is) {}

  std::ostringstream ss;
  indenting_ostream is;
  log::simple_summary logger;
};

suite<logger_factory> test_simple_summary("simple summary logger", [](auto &_) {
  _.test("passing run", [](logger_factory &f) {
    passing_run(f.logger);
    f.logger.summarize();
    expect(f.ss.str(), equal_to(
      "3/3 tests passed\n"
    ));
  });

  _.test("failing run", [](logger_factory &f) {
    failing_run(f.logger);
    f.logger.summarize();
    expect(f.ss.str(), equal_to(
      "1/3 tests passed (1 skipped)\n"
      "  suite > subsuite > test SKIPPED\n"
      "    message\n"
      "    more\n"
      "  second suite > test FAILED\n"
      "    error\n"
      "    more\n"
    ));
  });
});
