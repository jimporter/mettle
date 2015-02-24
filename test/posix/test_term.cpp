#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/log/term.hpp>

suite<> test_term("terminal output", [](auto &_) {
  using namespace mettle::term;

  _.test("reset", []() {
    std::ostringstream ss;
    enable(ss, true);
    ss << reset();
    expect(ss.str(), equal_to("\033[0m"));
  });

  _.test("foreground color", []() {
    std::ostringstream ss;
    enable(ss, true);
    ss << format(fg(color::red)) << "text" << reset();
    expect(ss.str(), equal_to("\033[31mtext\033[0m"));
  });

  _.test("background color", []() {
    std::ostringstream ss;
    enable(ss, true);
    ss << format(bg(color::red)) << "text" << reset();
    expect(ss.str(), equal_to("\033[41mtext\033[0m"));
  });

  _.test("foreground color, bold", []() {
    std::ostringstream ss;
    enable(ss, true);
    ss << format(sgr::bold, fg(color::red)) << "text" << reset();
    expect(ss.str(), equal_to("\033[1;31mtext\033[0m"));
  });

  _.test("background color, bold", []() {
    std::ostringstream ss;
    enable(ss, true);
    ss << format(sgr::bold, bg(color::red)) << "text" << reset();
    expect(ss.str(), equal_to("\033[1;41mtext\033[0m"));
  });

  _.test("disabled", []() {
    std::ostringstream ss;
    enable(ss, false);
    ss << format(sgr::bold, fg(color::red)) << "text" << reset();
    expect(ss.str(), equal_to("text"));
  });
});
