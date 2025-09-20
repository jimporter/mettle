#include <mettle/driver/log/term.hpp>

#include <cassert>
#include <cstdlib>
#include <string>

#include <windows.h>

namespace mettle::term {

  namespace {
    int enabled_flag = std::ios_base::xalloc();
    int console_flag = std::ios_base::xalloc();

    inline int current_attrs(std::ios_base &ios) {
      CONSOLE_SCREEN_BUFFER_INFO info;
      GetConsoleScreenBufferInfo(ios.pword(console_flag), &info);
      return info.wAttributes;
    }

    inline int default_attrs(std::ios_base &ios) {
      return ios.iword(console_flag);
    }

    inline int default_fg(std::ios_base &ios) {
      return ios.iword(console_flag) & 0x0f;
    }

    inline int default_bg(std::ios_base &ios) {
      return ios.iword(console_flag) & 0xf0;
    }

    int ansi_to_win_fg(int val) {
      switch(val) {
      case 30: return 0;
      case 31: return FOREGROUND_RED;
      case 32: return FOREGROUND_GREEN;
      case 33: return FOREGROUND_RED | FOREGROUND_GREEN;
      case 34: return FOREGROUND_BLUE;
      case 35: return FOREGROUND_RED | FOREGROUND_BLUE;
      case 36: return FOREGROUND_GREEN | FOREGROUND_BLUE;
      case 37: return FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
      default: assert(false && "disallowed color value"); std::abort();
      }
    }

    int ansi_to_win_bg(int val) {
      switch(val) {
      case 40: return 0;
      case 41: return BACKGROUND_RED;
      case 42: return BACKGROUND_GREEN;
      case 43: return BACKGROUND_RED | BACKGROUND_GREEN;
      case 44: return BACKGROUND_BLUE;
      case 45: return BACKGROUND_RED | BACKGROUND_BLUE;
      case 46: return BACKGROUND_GREEN | BACKGROUND_BLUE;
      case 47: return BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
      default: assert(false && "disallowed color value"); std::abort();
      }
    }

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

  void enable(std::ios_base &ios, bool enabled) {
    if((ios.iword(enabled_flag) != 0) == enabled)
      return;

    if(enabled) {
      // XXX: Make sure we're actually talking on CONOUT$.
      HANDLE handle = dup(GetStdHandle(STD_OUTPUT_HANDLE));
      CONSOLE_SCREEN_BUFFER_INFO info;
      GetConsoleScreenBufferInfo(handle, &info);

      ios.iword(console_flag) = info.wAttributes;
      ios.pword(console_flag) = handle;
      ios.register_callback([](std::ios_base::event type, std::ios_base &ios,
                               int flag) {
        switch(type) {
        case std::ios_base::copyfmt_event:
          ios.pword(flag) = dup(GetStdHandle(STD_OUTPUT_HANDLE));
          break;
        case std::ios_base::erase_event:
          CloseHandle(ios.pword(flag));
          break;
        }
      }, console_flag);
    } else {
      CloseHandle(ios.pword(console_flag));
    }

    ios.iword(enabled_flag) = enabled;
  }

  bool is_enabled(std::ios_base &ios) {
    return ios.iword(enabled_flag);
  }

  std::ostream & operator <<(std::ostream &o, const format &fmt) {
    if(!o.iword(enabled_flag))
      return o;

    WORD value = current_attrs(o);
    for(auto i : fmt.values_) {
      if(i == sgr::reset) {
        value = default_attrs(o);
      } else if(i == sgr::bold) {
        value |= 0x08;
      } else {
        int val = static_cast<int>(i);
        if(val >= 30 && val < 38) {        // Foreground
          value &= 0xf8;
          value |= ansi_to_win_fg(val);
        } else if(val == 39) {             // Default foreground
          value &= 0xf8;
          value |= default_fg(o);
        } else if(val >= 40 && val < 48) { // Background
          value &= 0x8f;
          value |= ansi_to_win_bg(val);
        } else if(val == 49) {             // Default background
          value &= default_fg(o);
        }
      }
    }

    SetConsoleTextAttribute(o.pword(console_flag), value);
    return o;
  }

  std::ostream & operator <<(std::ostream &o, const link &fmt) {
    // Not supported on Windows console.
    return o;
  }

  std::string file_url(const std::string &file_name, int line) {
    // Not supported on Windows console.
    return "";
  }

} // namespace mettle::term
