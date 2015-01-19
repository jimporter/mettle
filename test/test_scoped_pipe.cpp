#include <mettle.hpp>
using namespace mettle;

#include <iostream>

#include <mettle/driver/posix/scoped_pipe.hpp>

suite<scoped_pipe> test_scoped_pipe("scoped_pipe", [](auto &_) {
  _.setup([](scoped_pipe &pipe) {
    expect("open pipe", pipe.open(), equal_to(0));
  });

  _.test("open()", [](scoped_pipe &pipe) {
    expect("read fd", pipe.read_fd, greater_equal(0));
    expect("write fd", pipe.write_fd, greater_equal(0));
  });

  _.test("close()", [](scoped_pipe &pipe) {
    expect("close read fd", pipe.close_read(), equal_to(0));
    expect("read fd", pipe.read_fd, equal_to(-1));
    expect("close read fd again", pipe.close_read(), equal_to(-1));

    expect("close write fd", pipe.close_write(), equal_to(0));
    expect("write fd", pipe.write_fd, equal_to(-1));
    expect("close write fd again", pipe.close_write(), equal_to(-1));
  });

  _.test("read()/write()", [](scoped_pipe &pipe) {
    char in_data[] = "hello";
    expect(write(pipe.write_fd, in_data, sizeof(in_data)),
           equal_to(sizeof(in_data)));

    char out_data[sizeof(in_data)];
    expect(read(pipe.read_fd, out_data, sizeof(out_data)),
           equal_to(sizeof(in_data)));

    expect(std::string(out_data), equal_to(in_data));
  });

  // These might be a bit dangerous, since we're clobbering stdin/stdout...

  _.test("move_read()", [](scoped_pipe &pipe) {
    pipe.move_read(STDIN_FILENO);
    char in_data[] = "hello";
    expect(write(pipe.write_fd, in_data, sizeof(in_data) - 1),
           equal_to(sizeof(in_data) - 1));
    pipe.close_write();

    std::string out_data;
    std::cin >> out_data;
    expect(std::string(out_data), equal_to(in_data));
  });

  _.test("move_write()", [](scoped_pipe &pipe) {
    pipe.move_write(STDOUT_FILENO);
    char in_data[] = "hello";
    std::cout << in_data << std::flush;
    close(STDOUT_FILENO);

    char out_data[sizeof(in_data)];
    expect(read(pipe.read_fd, out_data, sizeof(out_data)),
           equal_to(sizeof(in_data) - 1));
    expect(std::string(out_data), equal_to(in_data));
  });
});
