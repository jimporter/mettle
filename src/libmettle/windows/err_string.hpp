#ifndef INC_METTLE_SRC_LIBMETTLE_WINDOWS_ERR_STRING_HPP
#define INC_METTLE_SRC_LIBMETTLE_WINDOWS_ERR_STRING_HPP

#include <string>

#include <windows.h>

namespace mettle {

namespace windows {

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

}

} // namespace mettle

#endif
