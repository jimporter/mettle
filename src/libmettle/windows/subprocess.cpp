#include <mettle/driver/windows/subprocess.hpp>

#include <cassert>
#include <cstdint>
#include <memory>

#include <windows.h>

#include <mettle/driver/windows/scoped_handle.hpp>

namespace mettle::windows {

  namespace {
    const std::size_t BUFSIZE = 4096;

    void cancel_io(const std::vector<readhandle> &dests) {
      for(const auto &i : dests)
        CancelIo(i.handle);
    }
  }

  HANDLE read_into(std::vector<readhandle> &dests, DWORD timeout,
                   const std::vector<HANDLE> &interrupts) {
    assert(!interrupts.empty());

    std::size_t total_size = dests.size() + interrupts.size();
    auto read_events = std::make_unique<scoped_handle[]>(dests.size());
    auto events = std::make_unique<HANDLE[]>(total_size);
    auto overlapped = std::make_unique<OVERLAPPED[]>(dests.size());
    auto bufs = std::make_unique<char[][BUFSIZE]>(dests.size());
    DWORD nread;

    for(std::size_t i = 0; i != dests.size(); i++) {
      if(!(read_events[i] = CreateEvent(nullptr, true, true, nullptr)))
        return nullptr;
      overlapped[i].hEvent = events[i] = read_events[i];
    }
    for(std::size_t i = 0; i != interrupts.size(); i++)
      events[dests.size() + i] = interrupts[i];

    for(std::size_t i = 0; i != dests.size(); i++) {
      if(!ReadFile(dests[i].handle, bufs[i], sizeof(bufs[i]), &nread,
                   &overlapped[i])) {
        auto err = GetLastError();
        if(err != ERROR_IO_PENDING && err != ERROR_BROKEN_PIPE) {
          cancel_io(dests);
          return nullptr;
        }
      }
    }

    while(true) {
      auto result = WaitForMultipleObjects(
        static_cast<DWORD>(total_size), events.get(), false, timeout
      );
      if(result == WAIT_FAILED || result == WAIT_TIMEOUT) {
        cancel_io(dests);
        return nullptr;
      }

      auto i = result - WAIT_OBJECT_0;
      assert(i < total_size);
      if(i >= dests.size()) {
        cancel_io(dests);
        return interrupts[i - dests.size()];
      }

      if(!GetOverlappedResult(dests[i].handle, &overlapped[i], &nread, true)) {
        switch(GetLastError()) {
        case ERROR_BROKEN_PIPE:
          // Our pipe broke; stop listening for its event.
          ResetEvent(events[i]);
          continue;
        case ERROR_IO_INCOMPLETE:
          continue;
        default:
          cancel_io(dests);
          return nullptr;
        }
      }

      dests[i].dest->append(bufs[i], nread);
      if(!ReadFile(dests[i].handle, bufs[i], sizeof(bufs[i]), &nread,
                   &overlapped[i])) {
        auto err = GetLastError();
        if(err != ERROR_IO_PENDING && err != ERROR_BROKEN_PIPE) {
          cancel_io(dests);
          return nullptr;
        }
      }
    }
  }

} // namespace mettle::windows
