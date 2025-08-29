#include <mettle/driver/log/term.hpp>

#include <cassert>
#include <string>

#include <mettle/detail/algorithm.hpp>

namespace mettle::term {

  namespace {
    int enabled_flag = std::ios_base::xalloc();
  }

  void enable(std::ios_base &ios, bool enabled) {
    ios.iword(enabled_flag) = enabled;
  }

  bool is_enabled(std::ios_base &ios) {
    return ios.iword(enabled_flag);
  }

  std::ostream & operator <<(std::ostream &o, const format &fmt) {
    assert(!fmt.values_.empty());
    if(!o.iword(enabled_flag))
      return o;
    return o << "\033[" << mettle::detail::joined(fmt.values_, [](sgr s) {
      // Use to_string() to ensure that we output in decimal, no matter what the
      // stream's state.
      return std::to_string(static_cast<int>(s));
    }, ";") << "m";
  }

} // namespace mettle::term
