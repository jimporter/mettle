#ifndef INC_METTLE_SRC_UTILS_HPP
#define INC_METTLE_SRC_UTILS_HPP

#include <cstring>
#include <string>

namespace mettle {

inline std::string err_string(int errnum) {
  char buf[256];
#ifdef _GNU_SOURCE
  return strerror_r(errnum, buf, sizeof(buf));
#else
  if(strerror_r(errnum, buf, sizeof(buf)) < 0)
    return "";
  return buf;
#endif
}

} // namespace mettle

#endif
