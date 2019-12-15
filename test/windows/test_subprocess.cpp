#include <mettle.hpp>
using namespace mettle;

#include <iostream>
#include <thread>

#include <mettle/driver/exit_code.hpp>
#include <mettle/driver/windows/scoped_pipe.hpp>
#include <mettle/driver/windows/subprocess.hpp>
using namespace mettle::windows;

#include "last_error.hpp"

struct read_into_fixture {
  scoped_pipe pipe[2];
  std::string results[2];
  std::vector<readhandle> readhandles;
};

void sighandler(int) {}

suite<read_into_fixture> test_read_into("read_into()", [](auto &_) {
  _.setup([](read_into_fixture &f) {
    expect("open pipe 1", f.pipe[0].open(true, false), equal_to(true));
    expect("open pipe 2", f.pipe[1].open(true, false), equal_to(true));

    f.readhandles = {
      {f.pipe[0].read_handle, &f.results[0]},
      {f.pipe[1].read_handle, &f.results[1]}
    };
  });

  _.test("read until handles close", [](read_into_fixture &f) {
    std::thread t([&f]() {
      DWORD written;
      WriteFile(f.pipe[0].write_handle, "pipe 1", 6, &written, nullptr);
      WriteFile(f.pipe[1].write_handle, "pipe 2", 6, &written, nullptr);

      f.pipe[0].close_write();
      f.pipe[1].close_write();
    });
    std::vector<HANDLE> interrupts = {t.native_handle()};

    expect(read_into(f.readhandles, INFINITE, interrupts),
           not_equal_to(nullptr));
    t.join();

    expect(f.results[0], equal_to("pipe 1"));
    expect(f.results[1], equal_to("pipe 2"));
  });

  _.test("read until timeout", [](read_into_fixture &f) {
    std::thread t([&f]() {
      DWORD written;
      WriteFile(f.pipe[0].write_handle, "pipe 1", 6, &written, nullptr);
      std::this_thread::sleep_for(std::chrono::milliseconds(250));
      WriteFile(f.pipe[1].write_handle, "pipe 2", 6, &written, nullptr);
    });
    std::vector<HANDLE> interrupts = {t.native_handle()};
    t.detach();

    expect(read_into(f.readhandles, 10, interrupts),
           equal_to(nullptr));

    expect(f.results[0], equal_to("pipe 1"));
    expect(f.results[1], equal_to(""));
  });
});
