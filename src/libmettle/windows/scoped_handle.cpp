#include <mettle/driver/windows/scoped_handle.hpp>

#include <windows.h>

namespace mettle {

namespace windows {

  bool scoped_handle::close() {
    if(!handle_) {
      SetLastError(ERROR_INVALID_HANDLE);
      return false;
    }

    bool success = CloseHandle(handle_) != 0;
    if(success)
      handle_ = nullptr;
    return success;
  }

}

} // namespace mettle
