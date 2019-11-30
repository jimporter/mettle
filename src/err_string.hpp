#ifndef INC_METTLE_SRC_ERR_STRING_HPP
#define INC_METTLE_SRC_ERR_STRING_HPP

#include <string>

#ifndef _WIN32
#  include <cstring>
#else
#  include <windows.h>
#endif

namespace mettle {

#ifndef _WIN32

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

#else

  inline std::string err_string(DWORD errnum) {
    char *tmp;
    FormatMessageA(
      FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM |
      FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, errnum, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<char*>(&tmp), 0, nullptr
    );
    std::string msg(tmp);
    LocalFree(tmp);
    return msg;
  }

#endif

} // namespace mettle

#endif
