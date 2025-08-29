#include <mettle/driver/log/format.hpp>

#include <mettle/driver/log/term.hpp>

namespace mettle {

  std::ostream & operator <<(std::ostream &os, const test_failure &failure) {
    using namespace term;

    if(!failure.desc.empty())
      os << failure.desc;
    if(!failure.desc.empty() && !failure.file_name.empty())
      os << " (";
    if(!failure.file_name.empty()) {
      os << link(file_url(failure.file_name, failure.line))
         << failure.file_name;
      if (failure.line)
        os << ":" << failure.line;
      os << link();
    }
    if(!failure.desc.empty() && !failure.file_name.empty())
      os << ")";

    if(!failure.desc.empty() || !failure.file_name.empty())
      os << std::endl;

    return os << failure.message;
  }

} // namespace mettle
