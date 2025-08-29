#ifndef INC_METTLE_DRIVER_LOG_TERM_HPP
#define INC_METTLE_DRIVER_LOG_TERM_HPP

#include <cstdint>
#include <ostream>
#include <type_traits>
#include <vector>

#include "../detail/export.hpp"

namespace mettle::term {

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

  class format {
    friend METTLE_PUBLIC std::ostream &
    operator <<(std::ostream &, const format &);
  public:
    template<typename ...Args>
    explicit format(Args &&...args) : values_{std::forward<Args>(args)...} {
      static_assert(sizeof...(Args) > 0,
                    "term::format must have at least one argument");
      static_assert(std::conjunction_v<std::is_same<sgr, Args>...>,
                    "term::format's arguments must be of type term::sgr");
    }
  private:
    std::vector<sgr> values_;
  };

  inline format reset() {
    return format(sgr::reset);
  }

  METTLE_PUBLIC void enable(std::ios_base &ios, bool enabled);
  METTLE_PUBLIC bool is_enabled(std::ios_base &ios);

} // namespace mettle::term

#endif
