#include <mettle.hpp>
using namespace mettle;

#include <memory>

#include "last_error.hpp"
#include <mettle/driver/windows/scoped_handle.hpp>
using namespace mettle::windows;

suite<std::unique_ptr<scoped_handle>>
test_scoped_handle("windows::scoped_handle", [](auto &_) {
  _.setup([](auto &handle) {
    handle = std::make_unique<scoped_handle>(
      CreateMutexA(nullptr, false, nullptr)
    );
  });

  _.test("handle conversion", [](auto &handle) {
    DWORD info;
    expect("get handle info", GetHandleInformation(*handle, &info),
           equal_to(TRUE));
  });

  _.test("~scoped_handle()", [](auto &handle) {
    HANDLE orig_handle = handle->handle();
    handle.reset();

    DWORD info;
    expect("get handle info", GetHandleInformation(orig_handle, &info),
           all( equal_to(FALSE), equal_last_error(ERROR_INVALID_HANDLE) ));
  });
});
