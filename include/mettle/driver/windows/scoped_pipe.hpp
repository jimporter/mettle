#ifndef INC_METTLE_DRIVER_WINDOWS_SCOPED_PIPE_HPP
#define INC_METTLE_DRIVER_WINDOWS_SCOPED_PIPE_HPP

#include <utility>

#include "scoped_handle.hpp"
#include "../detail/export.hpp"

namespace mettle {

namespace windows {

  class METTLE_PUBLIC scoped_pipe {
  public:
    scoped_pipe() = default;
    scoped_pipe(const scoped_pipe &) = delete;

    bool open(bool overlapped_read, bool overlapped_write);

    bool open() {
      return open(false, false);
    }

    bool set_read_inherit(bool inherit) {
      return do_set_inherit(read_handle, inherit);
    }

    bool set_write_inherit(bool inherit) {
      return do_set_inherit(write_handle, inherit);
    }

    bool close_read() {
      return read_handle.close();
    }

    bool close_write() {
      return write_handle.close();
    }

    scoped_handle read_handle, write_handle;
  private:
    bool do_set_inherit(HANDLE &handle, bool inherit);
  };

}

} // namespace mettle

#endif
