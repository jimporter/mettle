#ifndef INC_METTLE_DRIVER_LOG_TERM_HPP
#define INC_METTLE_DRIVER_LOG_TERM_HPP

#include <sstream>
#include <string>

namespace term {

namespace detail {
  inline int stream_flag() {
    static int flag = std::ios_base::xalloc();
    return flag;
  }
}

inline void enable(std::ios_base &ios, bool enabled) {
  ios.iword(detail::stream_flag()) = enabled;
}

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

inline size_t fg(const color &c) {
  return 30 + static_cast<size_t>(c);
}

inline size_t bg(const color &c) {
  return 40 + static_cast<size_t>(c);
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

class format {
  friend std::ostream & operator <<(std::ostream &, const format &);
public:
  template<typename First>
  explicit format(First &&first) {
    std::ostringstream ss;
    ss << "\033[" << static_cast<size_t>(std::forward<First>(first)) << "m";
    string_ = ss.str();
  }

  template<typename First, typename ...Rest>
  explicit format(First &&first, Rest &&...rest) {
    std::ostringstream ss;
    ss << "\033[" << static_cast<size_t>(std::forward<First>(first));

    size_t args[] = {static_cast<size_t>(std::forward<Rest>(rest))...};
    for(const auto &i : args)
      ss << ";" << i;
    ss << "m";
    string_ = ss.str();
  }
private:
  std::string string_;
};

inline format reset() {
  return format(sgr::reset);
}

inline std::ostream & operator <<(std::ostream &o, const format &fmt) {
  if(o.iword(detail::stream_flag()))
    return o << fmt.string_;
  return o;
}

} // namespace term

#endif
