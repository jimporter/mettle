#include <mettle/driver/log/format.hpp>

namespace mettle {

  std::ostream & operator <<(std::ostream &os, const test_failure &failure) {
    if(!failure.desc.empty())
      os << failure.desc;
    if(!failure.desc.empty() && !failure.file_name.empty())
      os << " (";
    if(!failure.file_name.empty())
      os << failure.file_name << ":" << failure.line;
    if(!failure.desc.empty() && !failure.file_name.empty())
      os << ")";

    if(!failure.desc.empty() || !failure.file_name.empty())
      os << std::endl;

    return os << failure.message;
  }

} // namespace mettle
