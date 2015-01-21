#ifndef INC_METTLE_DRIVER_WINDOWS_SCOPED_HANDLE_HPP
#define INC_METTLE_DRIVER_WINDOWS_SCOPED_HANDLE_HPP

#include <cassert>

#include <wtypes.h>

#include "../detail/export.hpp"

namespace mettle {

namespace windows {

  class METTLE_PUBLIC scoped_handle {
  public:
    scoped_handle(HANDLE handle = nullptr) : handle_(handle) {}
    scoped_handle(const scoped_handle &) = delete;

    ~scoped_handle() {
      close();
    }

    scoped_handle & operator =(HANDLE handle) {
      assert(handle_ == nullptr);
      handle_ = handle;
      return *this;
    }

    bool close();

    const HANDLE & handle() const {
      return handle_;
    }

    HANDLE & handle() {
      return handle_;
    }

    operator const HANDLE &() const {
      return handle_;
    }

    operator HANDLE &() {
      return handle_;
    }

    explicit operator bool() const {
      return handle_ != nullptr;
    }
  private:
    HANDLE handle_;
  };

}

} // namespace mettle

#endif
