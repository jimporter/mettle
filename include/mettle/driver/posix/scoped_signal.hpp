#ifndef INC_METTLE_DRIVER_POSIX_SCOPED_SIGNAL_HPP
#define INC_METTLE_DRIVER_POSIX_SCOPED_SIGNAL_HPP

#include <signal.h>

#include <cassert>
#include <initializer_list>
#include <vector>

namespace mettle::posix {

  class scoped_sigprocmask {
  public:
    scoped_sigprocmask() = default;
    scoped_sigprocmask(const scoped_sigprocmask &) = delete;

    ~scoped_sigprocmask() {
      clear();
    }

    template<typename T>
    int push(int how, const T &t) {
      return do_push(how, t);
    }

    int push(int how, std::initializer_list<int> t) {
      return do_push(how, t);
    }

    int push(int how, int signum) {
      return push(how, {signum});
    }

    int pop() {
      assert(!old_masks_.empty());
      int err;
      if((err = sigprocmask(SIG_SETMASK, &old_masks_.back(), nullptr)) < 0)
        return err;

      old_masks_.pop_back();
      return 0;
    }

    int clear() {
      if(!old_masks_.empty()) {
        int err;
        if((err = sigprocmask(SIG_SETMASK, &old_masks_.front(), nullptr)) < 0)
          return err;
      }

      old_masks_.clear();
      return 0;
    }
  private:
    template<typename T>
    int do_push(int how, const T &t) {
      int err;
      sigset_t mask;
      sigemptyset(&mask);
      for(auto &&signum : t) {
        if((err = sigaddset(&mask, signum)) < 0)
          return err;
      }

      try {
        old_masks_.emplace_back();
      } catch(...) {
        errno = ENOMEM;
        return -1;
      }

      if((err = sigprocmask(how, &mask, &old_masks_.back())) < 0)
        old_masks_.pop_back();
      return err;
    }

    std::vector<sigset_t> old_masks_;
  };

  class scoped_sigaction {
  public:
    using function_type = void (*)(int);

    scoped_sigaction() = default;
    scoped_sigaction(const scoped_sigaction &) = delete;

    ~scoped_sigaction() {
      close();
    }

    int open(int sig, function_type handler) {
      assert(signum == 0);
      signum = sig;
      sigemptyset(&act.sa_mask);
      act.sa_flags = 0;
      act.sa_handler = handler;
      return sigaction(signum, &act, &old_act);
    }

    int close() {
      if(signum == 0) {
        errno = EINVAL;
        return -1;
      }

      int err;
      err = sigaction(signum, &old_act, nullptr);
      signum = 0;
      return err;
    }
  private:
    int signum = 0;
    struct sigaction act, old_act;
  };

} // namespace mettle::posix

#endif
