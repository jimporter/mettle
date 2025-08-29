#include <mettle/driver/log/term.hpp>

#include <cassert>
#include <optional>
#include <string>

#include <unistd.h>

#include <mettle/detail/algorithm.hpp>

namespace mettle::term {

  namespace {
    std::optional<std::string> hostname;

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

  std::ostream & operator <<(std::ostream &o, const link &fmt) {
    if(!o.iword(enabled_flag))
      return o;
    return o << "\033]8;;" << fmt.url_ << "\033\\";
  }

  std::string file_url(const std::string &file_name, int line) {
    if(file_name.empty())
      return "";

    if(!hostname) {
      char buf[1024] = "";
      gethostname(buf, sizeof(buf) - 1);
      hostname = buf;
    }

    // Some terminals support including the line number in the URI fragment. See
    // <https://iterm2.com/documentation-escape-codes.html>.
    std::string url = "file://" + *hostname + file_name;
    if(line)
      url += "#" + std::to_string(line);
    return url;
  }


} // namespace mettle::term
