#include <mettle.hpp>
using namespace mettle;

#include <cstdlib>

suite<> test_suite("suite", [](auto &) {
  abort();
});
