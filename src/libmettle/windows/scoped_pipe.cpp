#include <mettle/driver/windows/scoped_pipe.hpp>

#include <cassert>
#include <cstdint>
#include <sstream>

namespace mettle {

namespace windows {

  bool scoped_pipe::open(const std::pair<bool, bool> &overlapped) {
    assert(!read_handle && !write_handle);
    const std::size_t BUFSIZE = 4096;
    static std::uint64_t pipe_id = 0;

    std::ostringstream os;
    os << "\\\\.\\pipe\\anonpipe." << GetCurrentProcessId() << "." << pipe_id++;
    const char *name = os.str().c_str();

    DWORD read_flags = PIPE_ACCESS_INBOUND;
    if(overlapped.first)
      read_flags |= FILE_FLAG_OVERLAPPED;
    read_handle = CreateNamedPipeA(
      name, read_flags, 0, 1, BUFSIZE, BUFSIZE, 0, nullptr
    );
    if(!read_handle)
      return false;

    DWORD write_flags = FILE_ATTRIBUTE_NORMAL;
    if(overlapped.first)
      write_flags |= FILE_FLAG_OVERLAPPED;
    write_handle = CreateFileA(
      name, GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, write_flags, nullptr
    );
    if(write_handle == INVALID_HANDLE_VALUE) {
      auto last_err = GetLastError();
      read_handle.close();
      SetLastError(last_err);

      read_handle = nullptr;
      write_handle = nullptr;
      return false;
    }

    return true;
  }

  bool scoped_pipe::do_set_inherit(HANDLE &handle, bool inherit) {
    if(!handle) {
      SetLastError(ERROR_INVALID_HANDLE);
      return false;
    }

    return SetHandleInformation(
      handle, HANDLE_FLAG_INHERIT, inherit ? HANDLE_FLAG_INHERIT : 0
    ) != 0;
  }

}

} // namespace mettle
