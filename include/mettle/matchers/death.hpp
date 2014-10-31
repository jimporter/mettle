#ifndef INC_METTLE_MATCHERS_DEATH_HPP
#define INC_METTLE_MATCHERS_DEATH_HPP

#include "core.hpp"
#include "result.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include <system_error>

namespace mettle {

namespace detail {
  template<typename T, typename U>
  int get_status(T &&func, U &&terminate) {
    pid_t pid;
    if((pid = fork()) < 0)
      throw std::system_error(errno, std::system_category());
    if(pid == 0) {
      try {
        func();
      }
      catch(...) {}
      terminate();
      assert(false && "should never get here");
    }
    else {
      int status;
      if(waitpid(pid, &status, 0) < 0)
        throw std::system_error(errno, std::system_category());
      // Make sure the child is dead.
      if(WIFSTOPPED(status))
        kill(pid, SIGKILL);
      return status;
    }
  }

  template<typename Matcher>
  class killed_impl : public matcher_tag {
  public:
    template<typename T>
    killed_impl(T &&t, bool verbose = true)
      : matcher_(std::forward<T>(t)), verbose_(verbose) {}

    template<typename U>
    match_result operator ()(U &&value) const {
      int status = get_status(std::forward<U>(value), terminate);
      if(WIFSIGNALED(status)) {
        std::ostringstream ss;
        ss << "killed with signal " << WTERMSIG(status);
        return { matcher_(WTERMSIG(status)), ss.str() };
      }
      return { false, "wasn't killed" };
    }

    std::string desc() const {
      std::ostringstream ss;
      ss << "killed";
      if (verbose_)
        ss << " with signal " << matcher_.desc();
      return ss.str();
    }
  private:
    [[noreturn]] static void terminate() {
      _exit(0);
    }

    Matcher matcher_;
    bool verbose_;
  };

  template<typename Matcher>
  class exited_impl : public matcher_tag {
  public:
    template<typename T>
    exited_impl(T &&t, bool verbose = true)
      : matcher_(std::forward<T>(t)), verbose_(verbose) {}

    template<typename U>
    match_result operator ()(U &&value) const {
      int status = get_status(std::forward<U>(value), terminate);
      if(WIFEXITED(status)) {
        std::ostringstream ss;
        ss << "exited with status " << WEXITSTATUS(status);
        return { matcher_(WEXITSTATUS(status)), ss.str() };
      }
      return { false, "didn't exit" };
    }

    std::string desc() const {
      std::ostringstream ss;
      ss << "exited";
      if (verbose_)
        ss << " with status " << matcher_.desc();
      return ss.str();
    }
  private:
    [[noreturn]] static void terminate() {
      abort();
    }

    Matcher matcher_;
    bool verbose_;
  };
}

inline auto killed() {
  return detail::killed_impl<decltype(anything())>(anything(), false);
}

template<typename T>
inline auto killed(T &&thing) {
  auto matcher = ensure_matcher(std::forward<T>(thing));
  return detail::killed_impl<decltype(matcher)>(std::move(matcher));
}

inline auto exited() {
  return detail::exited_impl<decltype(anything())>(anything(), false);
}

template<typename T>
inline auto exited(T &&thing) {
  auto matcher = ensure_matcher(std::forward<T>(thing));
  return detail::exited_impl<decltype(matcher)>(std::move(matcher));
}

} // namespace mettle

#endif
