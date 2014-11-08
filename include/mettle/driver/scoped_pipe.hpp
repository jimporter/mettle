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

  int move_read(int new_fd) {
    return do_move(read_fd, new_fd);
  }

  int move_write(int new_fd) {
    return do_move(write_fd, new_fd);
  }

  int read_fd = -1, write_fd = -1;
private:
  int do_close(int &fd) {
    if(fd == -1) {
      errno = EBADF;
      return -1;
    }

    int err = ::close(fd);
    if(err == 0)
      fd = -1;
    return err;
  }

  int do_move(int &old_fd, int new_fd) {
    if(old_fd == -1) {
      errno = EBADF;
      return -1;
    }
    if(old_fd == new_fd)
      return 0;

    int err = 0;
    if((err = dup2(old_fd, new_fd)) < 0)
      return err;
    return do_close(old_fd);
  }
};

} // namespace mettle

#endif
