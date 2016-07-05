#ifndef INC_METTLE_DRIVER_WINDOWS_SUBPROCESS_HPP
#define INC_METTLE_DRIVER_WINDOWS_SUBPROCESS_HPP

#include <string>
#include <vector>

#include <wtypes.h>

#include "../detail/export.hpp"

namespace mettle {

namespace windows {
  struct readhandle {
    HANDLE handle;
    std::string *dest;
  };

  METTLE_PUBLIC HANDLE
  read_into(std::vector<readhandle> &dests, DWORD timeout,
            const std::vector<HANDLE> &interrupts);
}

} // namespace mettle

#endif
