#include <mettle.hpp>
using namespace mettle;

#include <iostream>
#include <memory>
#include <fcntl.h>
#include <unistd.h>

#include "errno.hpp"
#include <mettle/driver/posix/scoped_pipe.hpp>
using namespace mettle::posix;

suite<std::unique_ptr<scoped_pipe>>
test_scoped_pipe("scoped_pipe", [](auto &_) {
  _.setup([](auto &pipe) {
    pipe = std::make_unique<scoped_pipe>();
    expect("open pipe", pipe->open(), equal_to(0));
  });

  _.test("open()", [](auto &pipe) {
    expect("read fd", pipe->read_fd, greater_equal(0));
    expect("write fd", pipe->write_fd, greater_equal(0));

    expect("reopen pipe", [&pipe]() { pipe->open(); }, killed(SIGABRT));
  });

  _.test("close()", [](auto &pipe) {
    int read_fd, write_fd;
    read_fd = pipe->read_fd;
    write_fd = pipe->write_fd;

    expect("close read fd", pipe->close_read(), equal_to(0));
    expect("read fd", pipe->read_fd, equal_to(-1));
    expect("close read fd again", pipe->close_read(),
           all( equal_to(-1), equal_errno(EBADF) ));
    expect("get read fd flags", fcntl(read_fd, F_GETFD),
           all( equal_to(-1), equal_errno(EBADF) ));

    expect("close write fd", pipe->close_write(), equal_to(0));
    expect("write fd", pipe->write_fd, equal_to(-1));
    expect("close write fd again", pipe->close_write(),
           all( equal_to(-1), equal_errno(EBADF) ));
    expect("get write fd flags", fcntl(write_fd, F_GETFD),
           all( equal_to(-1), equal_errno(EBADF) ));
  });

  _.test("~scoped_pipe()", [](auto &pipe) {
    int read_fd = pipe->read_fd;
    int write_fd = pipe->write_fd;
    pipe.reset();

    expect("get read fd flags", fcntl(read_fd, F_GETFD),
           all( equal_to(-1), equal_errno(EBADF) ));
    expect("get write fd flags", fcntl(write_fd, F_GETFD),
           all( equal_to(-1), equal_errno(EBADF) ));
  });

  _.test("read()/write()", [](auto &pipe) {
    char in_data[] = "hello";
    expect("write data", write(pipe->write_fd, in_data, sizeof(in_data)),
           equal_to(sizeof(in_data)));

    char out_data[sizeof(in_data)];
    expect("read data", read(pipe->read_fd, out_data, sizeof(out_data)),
           equal_to(sizeof(in_data)));

    expect(std::string(out_data), equal_to(in_data));
  });

  // These might be a bit dangerous, since we're clobbering stdin/stdout...

  _.test("move_read()", [](auto &pipe) {
    pipe->move_read(STDIN_FILENO);
    char in_data[] = "hello";
    expect("write data", write(pipe->write_fd, in_data, sizeof(in_data) - 1),
           equal_to(sizeof(in_data) - 1));
    pipe->close_write();

    std::string out_data;
    std::cin >> out_data;
    expect(std::string(out_data), equal_to(in_data));
  });

  _.test("move_write()", [](auto &pipe) {
    pipe->move_write(STDOUT_FILENO);
    char in_data[] = "hello";
    std::cout << in_data << std::flush;
    close(STDOUT_FILENO);

    char out_data[sizeof(in_data)];
    expect("read data", read(pipe->read_fd, out_data, sizeof(out_data)),
           equal_to(sizeof(in_data) - 1));
    expect(std::string(out_data), equal_to(in_data));
  });
});
