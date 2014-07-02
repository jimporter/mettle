#ifndef INC_METTLE_TERM_HPP
#define INC_METTLE_TERM_HPP

#include <sstream>
#include <string>

namespace term {

enum class color {
  black   = 0,
  red     = 1,
  green   = 2,
  yellow  = 3,
  blue    = 4,
  magenta = 5,
  cyan    = 6,
  white   = 7
};

inline size_t fg(const color &c) {
  return 30 + static_cast<size_t>(c);
}

inline size_t bg(const color &c) {
  return 40 + static_cast<size_t>(c);
}

enum class sgr {
  normal    = 0,
  bold      = 1,
  underline = 4
};

class format {
  friend std::ostream & operator <<(std::ostream &, const format &);
public:
  template<typename First>
  explicit format(First &&first) {
    std::stringstream s;
    s << "\033[" << static_cast<size_t>(std::forward<First>(first)) << "m";
    string_ = s.str();
  }

  template<typename First, typename ...Rest>
  explicit format(First &&first, Rest &&...rest) {
    std::stringstream s;
    s << "\033[" << static_cast<size_t>(std::forward<First>(first));

    size_t args[] = {static_cast<size_t>(std::forward<Rest>(rest))...};
    for(const auto &i : args)
      s << ";" << i;
    s << "m";
    string_ = s.str();
  }
private:
  std::string string_;
};

inline format reset() {
  return format(sgr::normal);
}

bool colors_enabled = false;
inline std::ostream & operator <<(std::ostream &o, const format &fmt) {
  if(colors_enabled)
    return o << fmt.string_;
  return o;
}

} // namespace term

#endif
