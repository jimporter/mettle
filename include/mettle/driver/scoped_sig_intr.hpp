#ifndef INC_METTLE_DRIVER_SCOPED_SIG_INTR_HPP
#define INC_METTLE_DRIVER_SCOPED_SIG_INTR_HPP

#include <signal.h>

namespace mettle {

class scoped_sig_intr {
public:
  ~scoped_sig_intr() {
    close();
  }

  int open(int sig) {
    signum = sig;
    int err;

    sigemptyset(&mask);
    if((err = sigaddset(&mask, signum)) < 0 ||
       (err = sigprocmask(SIG_BLOCK, &mask, &old_mask)) < 0)
      return err;

    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = sig_intr;
    return sigaction(signum, &act, &old_act);
  }

  int close() {
    if(signum == 0) {
      errno = EINVAL;
      return -1;
    }
    int err;
    if((err = sigprocmask(SIG_SETMASK, &old_mask, nullptr)) < 0)
      return err;
    err = sigaction(signum, &old_act, nullptr);

    signum = 0;
    return err;
  }
private:
  static void sig_intr(int) {}

  int signum = 0;
  sigset_t mask, old_mask;
  struct sigaction act, old_act;
};

} // namespace mettle

#endif
