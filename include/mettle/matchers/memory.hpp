#ifndef INC_METTLE_MATCHERS_MEMORY_HPP
#define INC_METTLE_MATCHERS_MEMORY_HPP

#include "core.hpp"

namespace mettle {

  template<typename T>
  inline auto dereferenced(T &&thing) {
    return basic_matcher(
      ensure_matcher(std::forward<T>(thing)),
      [](auto &&actual, auto &&matcher) -> match_result {
        bool matched = false;
        std::ostringstream ss;
        ss << "-> ";

        if(!actual) {
          ss << to_printable(actual);
        } else {
          match_result m = matcher(*actual);
          matched = m.matched;
          if(m.message.empty())
            ss << to_printable(*actual);
          else
            ss << m.message;
        }

        return {matched, ss.str()};
      }, "-> "
    );
  }

} // namespace mettle

#endif
