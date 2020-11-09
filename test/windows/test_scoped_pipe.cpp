#include <mettle.hpp>
using namespace mettle;

#include <memory>

#include "last_error.hpp"
#include <mettle/driver/windows/scoped_pipe.hpp>
using namespace mettle::windows;

suite<std::unique_ptr<scoped_pipe>>
test_scoped_pipe("windows::scoped_pipe", [](auto &_) {
  _.setup([](auto &pipe) {
    pipe = std::make_unique<scoped_pipe>();
    expect("open pipe", pipe->open(), equal_to(true));
  });

  _.test("open()", [](auto &pipe) {
    expect("read handle", pipe->read_handle.handle(), not_equal_to(nullptr));
    expect("write handle", pipe->write_handle.handle(), not_equal_to(nullptr));
  });

  _.test("open(true, true)", [](auto &pipe) {
    pipe->close_read();
    pipe->close_write();

    expect("open overlapped pipe", pipe->open(true, true), equal_to(true));
    expect("read handle", pipe->read_handle.handle(), not_equal_to(nullptr));
    expect("write handle", pipe->write_handle.handle(), not_equal_to(nullptr));
  });

  _.test("set_*_inherit()", [](auto &pipe) {
    DWORD flags;

    GetHandleInformation(pipe->read_handle, &flags);
    expect("read handle flags", flags, equal_to(0));
    expect("set_read_inherit(true)", pipe->set_read_inherit(true),
           equal_to(true));
    GetHandleInformation(pipe->read_handle, &flags);
    expect("read handle flags", flags, equal_to(HANDLE_FLAG_INHERIT));
    expect("set_read_inherit(false)", pipe->set_read_inherit(false),
           equal_to(true));
    GetHandleInformation(pipe->read_handle, &flags);
    expect("read handle flags", flags, equal_to(0));

    GetHandleInformation(pipe->write_handle, &flags);
    expect("write handle flags", flags, equal_to(0));
    expect("set_write_inherit(true)", pipe->set_write_inherit(true),
           equal_to(true));
    GetHandleInformation(pipe->write_handle, &flags);
    expect("write handle flags", flags, equal_to(HANDLE_FLAG_INHERIT));
    expect("set_write_inherit(false)", pipe->set_write_inherit(false),
           equal_to(true));
    GetHandleInformation(pipe->write_handle, &flags);
    expect("write handle flags", flags, equal_to(0));
  });

  _.test("close()", [](auto &pipe) {
    expect("close read handle", pipe->close_read(), equal_to(true));
    expect("read handle", pipe->read_handle.handle(), equal_to(nullptr));
    expect("close read handle again", pipe->close_read(),
           all( equal_to(false), equal_last_error(ERROR_INVALID_HANDLE) ));

    expect("close write handle", pipe->close_write(), equal_to(true));
    expect("write handle", pipe->write_handle.handle(), equal_to(nullptr));
    expect("close write handle again", pipe->close_write(),
           all( equal_to(false), equal_last_error(ERROR_INVALID_HANDLE) ));
  });

  _.test("ReadFile()/WriteFile()", [](auto &pipe) {
    DWORD mode = PIPE_NOWAIT;
    expect("non-blocking write handle", SetNamedPipeHandleState(
      pipe->write_handle, &mode, nullptr, nullptr
    ), equal_to(TRUE));

    char in_data[] = "hello";
    DWORD written;
    expect("write data", WriteFile(
      pipe->write_handle, in_data, sizeof(in_data), &written, nullptr
    ), equal_to(TRUE));

    char out_data[sizeof(in_data)];
    DWORD read;
    expect("read data", ReadFile(
      pipe->read_handle, out_data, sizeof(out_data), &read, nullptr
    ), equal_to(TRUE));

    expect(std::string(out_data), equal_to(in_data));
  });
});
