#include <mettle.hpp>
using namespace mettle;

#include <cstdint>
#include <memory>
#include <system_error>
#include <vector>

#include <mettle/driver/posix/scoped_signal.hpp>
using namespace mettle::posix;

sigset_t getprocmask() {
  sigset_t mask;
  if(sigprocmask(0, nullptr, &mask) < 0)
    throw std::system_error(errno, std::system_category());
  return mask;
}

auto equal_sigprocmask(std::vector<int> mask) {
  using namespace detail;
  std::string desc = "[" + stringify(joined(mask, [](int i) {
    return to_printable(strsignal(i));
  })) + "]";

  return basic_matcher(
    [mask = std::move(mask)](const auto &actual) -> match_result {
      for(int sig : mask) {
        if(sigismember(&actual, sig) != 1) {
          std::ostringstream ss;
          ss << "no " << to_printable(strsignal(sig)) << " found";
          return {false, ss.str()};
        }
      }
      return true;
    }, desc
  );
}

suite<std::unique_ptr<scoped_sigprocmask>>
test_sigprocmask("posix::scoped_sigprocmask", [](auto &_) {
  _.setup([](auto &mask) {
    mask = std::make_unique<scoped_sigprocmask>();
    expect("push sigprocmask", mask->push(SIG_SETMASK, SIGTERM), equal_to(0));
    expect("get sigprocmask", getprocmask(), equal_sigprocmask({SIGTERM}));
  });

  _.test("push()/pop()", [](auto &mask) {
    expect("push sigprocmask", mask->push(SIG_BLOCK, {SIGINT, SIGQUIT}),
           equal_to(0));
    expect("get sigprocmask", getprocmask(),
           equal_sigprocmask({SIGTERM, SIGINT, SIGQUIT}));

    expect("pop sigprocmask", mask->pop(), equal_to(0));
    expect("get sigprocmask", getprocmask(), all(
      equal_sigprocmask({SIGTERM}),
      is_not(equal_sigprocmask({SIGINT})),
      is_not(equal_sigprocmask({SIGQUIT}))
    ));

    expect("pop sigprocmask", mask->pop(), equal_to(0));
    expect("get sigprocmask", getprocmask(), none(
      equal_sigprocmask({SIGTERM}),
      equal_sigprocmask({SIGINT}),
      equal_sigprocmask({SIGQUIT})
    ));
  });

  _.test("clear()", [](auto &mask) {
    expect("push sigprocmask", mask->push(SIG_SETMASK, SIGINT), equal_to(0));
    expect("get sigprocmask", getprocmask(), equal_sigprocmask({SIGINT}));

    expect("clear sigprocmask", mask->clear(), equal_to(0));
    expect("get sigprocmask", getprocmask(), none(
      equal_sigprocmask({SIGTERM}),
      equal_sigprocmask({SIGINT})
    ));
  });

  _.test("~scoped_sigprocmask()", [](auto &mask) {
    expect("push sigprocmask", mask->push(SIG_SETMASK, SIGTERM), equal_to(0));
    expect("get sigprocmask", getprocmask(), equal_sigprocmask({SIGTERM}));

    mask.reset();

    expect("get sigprocmask", getprocmask(),
           is_not(equal_sigprocmask({SIGTERM})));
  });
});

void sig_handler(int) {}

suite<std::unique_ptr<scoped_sigaction>>
test_sigaction("posix::scoped_sigaction", [](auto &_) {
  _.setup([](auto &act) {
    act = std::make_unique<scoped_sigaction>();
    expect("open sigaction", act->open(SIGTERM, sig_handler), equal_to(0));
  });

  _.test("open()", []([[maybe_unused]] auto &act) {
    struct sigaction old;
    expect("get sigaction", sigaction(SIGTERM, nullptr, &old), equal_to(0));
    expect("check handler", old.sa_handler, equal_to(sig_handler));

#ifndef NDEBUG
    expect("reopen sigaction", [&act]() { act->open(SIGTERM, sig_handler); },
           killed(SIGABRT));
#endif
  });

  _.test("close()", [](auto &act) {
    expect("close sigaction", act->close(), equal_to(0));

    struct sigaction old;
    expect("get sigaction", sigaction(SIGTERM, nullptr, &old), equal_to(0));
    expect("check handler", old.sa_handler, not_equal_to(sig_handler));
  });

  _.test("~scoped_sigaction()", [](auto &act) {
    act.reset();

    struct sigaction old;
    expect("get sigaction", sigaction(SIGTERM, nullptr, &old), equal_to(0));
    expect("check handler", old.sa_handler, not_equal_to(sig_handler));
  });
});
