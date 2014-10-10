#ifndef INC_METTLE_DRIVER_SCOPED_PIPE_HPP
#define INC_METTLE_DRIVER_SCOPED_PIPE_HPP

#include <unistd.h>

namespace mettle {

class scoped_pipe {
public:
  int open(int flags = 0) {
    return pipe2(&read_fd, flags);
  }

  ~scoped_pipe() {
    close_read();
    close_write();
  }

  int close_read() {
    return do_close(read_fd);
  }

  int close_write() {
    return do_close(write_fd);
  }

  int read_fd = -1, write_fd = -1;
private:
  int do_close(int &fd) {
    if(fd == -1)
      return 0;
    int err = ::close(fd);
    if(err == 0)
      fd = -1;
    return err;
  }
};

} // namespace mettle

#endif
