#ifndef INC_METTLE_DRIVER_LOG_TERM_HPP
#define INC_METTLE_DRIVER_LOG_TERM_HPP

#include <cstdint>
#include <sstream>
#include <string>
#include <type_traits>

#include "../detail/export.hpp"

#ifdef _WIN32
#  include <cassert>
#  include <vector>
#  include <windows.h>
#endif

namespace mettle {

namespace term {

  namespace detail {
    METTLE_PUBLIC inline int enabled_flag() {
      static int flag = std::ios_base::xalloc();
      return flag;
    }

    template<typename T, typename ...>
    struct are_same : std::true_type {};

    template<typename T, typename First, typename ...Rest>
    struct are_same<T, First, Rest...> : std::integral_constant<
      bool, std::is_same<T, First>::value && are_same<T, Rest...>::value
    > {};
  }

  enum class sgr {
    reset       = 0,
    bold        = 1,
    faint       = 2,
    italic      = 3,
    underline   = 4,
    blink       = 5,
    blink_fast  = 6,
    inverse     = 7,
    conceal     = 8,
    crossed_out = 9
  };

  enum class color {
    black   = 0,
    red     = 1,
    green   = 2,
    yellow  = 3,
    blue    = 4,
    magenta = 5,
    cyan    = 6,
    white   = 7,
    normal  = 9
  };

  inline sgr fg(color c) {
    return static_cast<sgr>(30 + static_cast<std::size_t>(c));
  }

  inline sgr bg(color c) {
    return static_cast<sgr>(40 + static_cast<std::size_t>(c));
  }

#ifndef _WIN32

  inline void enable(std::ios_base &ios, bool enabled) {
    ios.iword(detail::enabled_flag()) = enabled;
  }

  class format {
    friend std::ostream & operator <<(std::ostream &, const format &);
  public:
    template<typename ...Args>
    explicit format(Args &&...args) {
      static_assert(sizeof...(Args) > 0,
                    "term::format must have at least one argument");
      static_assert(detail::are_same<sgr, Args...>::value,
                    "term::format's arguments must be of type term::sgr");
      int values[] = {static_cast<int>(std::forward<Args>(args))...};

      std::ostringstream ss;
      ss << "\033[" << values[0];
      for(std::size_t i = 1; i != sizeof...(Args); i++)
        ss << ";" << values[i];
      ss << "m";
      string_ = ss.str();
    }
  private:
    void do_format(std::ostream &ios) const {
      ios << string_;
    }
    std::string string_;
  };

#else

  namespace detail {
    METTLE_PUBLIC inline int console_flag() {
      static int flag = std::ios_base::xalloc();
      return flag;
    }

    inline int current_attrs(std::ios_base &ios) {
      CONSOLE_SCREEN_BUFFER_INFO info;
      GetConsoleScreenBufferInfo(ios.pword(detail::console_flag()), &info);
      return info.wAttributes;
    }

    inline int default_attrs(std::ios_base &ios) {
      return ios.iword(detail::console_flag());
    }

    inline int default_fg(std::ios_base &ios) {
      return ios.iword(detail::console_flag()) & 0x0f;
    }

    inline int default_bg(std::ios_base &ios) {
      return ios.iword(detail::console_flag()) & 0xf0;
    }

// MSVC doesn't understand the [[noreturn]] attribute, so it thinks these can
// exit without returning.
#ifdef _MSC_VER
#  pragma warning(push)
#  pragma warning(disable:4715)
#endif

    inline int ansi_to_win_fg(int val) {
      switch(val) {
      case 30: return 0;
      case 31: return FOREGROUND_RED;
      case 32: return FOREGROUND_GREEN;
      case 33: return FOREGROUND_RED | FOREGROUND_GREEN;
      case 34: return FOREGROUND_BLUE;
      case 35: return FOREGROUND_RED | FOREGROUND_BLUE;
      case 36: return FOREGROUND_GREEN | FOREGROUND_BLUE;
      case 37: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
      default: assert(false && "disallowed color value");
      }
    }

    inline int ansi_to_win_bg(int val) {
      switch(val) {
      case 40: return 0;
      case 41: return BACKGROUND_RED;
      case 42: return BACKGROUND_GREEN;
      case 43: return BACKGROUND_RED | BACKGROUND_GREEN;
      case 44: return BACKGROUND_BLUE;
      case 45: return BACKGROUND_RED | BACKGROUND_BLUE;
      case 46: return BACKGROUND_GREEN | BACKGROUND_BLUE;
      case 47: return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
      default: assert(false && "disallowed color value");
      }
    }

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

    inline HANDLE dup(HANDLE h) {
      HANDLE proc = GetCurrentProcess();
      HANDLE result;
      if(!DuplicateHandle(proc, h, proc, &result, 0, true,
                          DUPLICATE_SAME_ACCESS)) {
        return nullptr;
      }
      return result;
    }
  }

  inline void enable(std::ios_base &ios, bool enabled) {
    int enabled_flag = detail::enabled_flag();
    int console_flag = detail::console_flag();

    if((ios.iword(enabled_flag) != 0) == enabled)
      return;

    if(enabled) {
      // XXX: Make sure we're actually talking on CONOUT$.
      HANDLE handle = detail::dup(GetStdHandle(STD_OUTPUT_HANDLE));
      CONSOLE_SCREEN_BUFFER_INFO info;
      GetConsoleScreenBufferInfo(handle, &info);

      ios.iword(console_flag) = info.wAttributes;
      ios.pword(console_flag) = handle;
      ios.register_callback([](std::ios_base::event type, std::ios_base &ios,
                               int flag) {
        switch(type) {
        case std::ios_base::copyfmt_event:
          ios.pword(flag) = detail::dup(GetStdHandle(STD_OUTPUT_HANDLE));
          break;
        case std::ios_base::erase_event:
          CloseHandle(ios.pword(flag));
          break;
        }
      }, console_flag);
    }
    else {
      CloseHandle(ios.pword(console_flag));
    }

    ios.iword(enabled_flag) = enabled;
  }

  class format {
    friend std::ostream & operator <<(std::ostream &, const format &);
  public:
    template<typename ...Args>
    explicit format(Args &&...args) {
      static_assert(sizeof...(Args) > 0,
                    "term::format must have at least one argument");
      static_assert(detail::are_same<sgr, Args...>::value,
                    "term::format's arguments must be of type term::sgr");
      values_ = {std::forward<Args>(args)...};
    }
  private:
    void do_format(std::ostream &o) const {
      WORD value = detail::current_attrs(o);
      for(auto i : values_) {
        if(i == sgr::reset) {
          value = detail::default_attrs(o);
        }
        else if(i == sgr::bold) {
          value |= 0x08;
        }
        else {
          int val = static_cast<int>(i);
          if(val >= 30 && val < 38) {      // Foreground
            value &= 0xf8;
            value |= detail::ansi_to_win_fg(val);
          }
          else if(val == 39) {             // Default foreground
            value &= 0xf8;
            value |= detail::default_fg(o);
          }
          else if(val >= 40 && val < 48) { // Background
            value &= 0x8f;
            value |= detail::ansi_to_win_bg(val);
          }
          else if(val == 49) {             // Default background
            value &= detail::default_fg(o);
          }
        }
      }

      SetConsoleTextAttribute(o.pword(detail::console_flag()), value);
    }

    std::vector<sgr> values_;
  };

#endif

  inline format reset() {
    return format(sgr::reset);
  }

  inline std::ostream & operator <<(std::ostream &o, const format &fmt) {
    if(o.iword(detail::enabled_flag()))
      fmt.do_format(o);
    return o;
  }

}

} // namespace mettle

#endif
