#ifndef INC_METTLE_DRIVER_WINDOWS_SUBPROCESS_HPP
#define INC_METTLE_DRIVER_WINDOWS_SUBPROCESS_HPP

#include <string>
#include <vector>

#include <wtypes.h>

namespace mettle {

namespace windows {

  struct readhandle {
    HANDLE handle;
    std::string *dest;
  };

  HANDLE read_into(std::vector<readhandle> &dests,
                   const std::vector<HANDLE> &interrupts);

}

} // namespace mettle

#endif
