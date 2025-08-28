#include <mettle/driver/subprocess_test_runner.hpp>

#include <iostream>
#include <memory>
#include <sstream>

#include <windows.h>

#include <mettle/detail/source_location.hpp>
#include <mettle/driver/exit_code.hpp>
#include <mettle/driver/windows/scoped_pipe.hpp>
#include <mettle/driver/windows/subprocess.hpp>

#include "../../err_string.hpp"

#ifdef METTLE_NO_SOURCE_LOCATION
#  define METTLE_FAILED() failed(                                             \
     ::mettle::detail::source_location::current(__FILE__, __func__, __LINE__) \
   )
#else
#  define METTLE_FAILED() failed()
#endif

namespace mettle {

  using namespace windows;

  namespace {
    test_result failed(detail::source_location loc =
                         detail::source_location::current()) {
      std::ostringstream ss;
      ss << "Fatal error at " << loc.file_name() << ":" << loc.line() << "\n"
         << err_string(GetLastError());
      return {false, ss.str()};
    }
  }

  test_result subprocess_test_runner::operator ()(
    const test_info &test, log::test_output &output
  ) const {
    scoped_pipe stdout_pipe, stderr_pipe, log_pipe;
    if(!stdout_pipe.open(true, false) ||
       !stderr_pipe.open(true, false) ||
       !log_pipe.open(true, false))
      return METTLE_FAILED();

    if(!stdout_pipe.set_write_inherit(true) ||
       !stderr_pipe.set_write_inherit(true) ||
       !log_pipe.set_write_inherit(true))
      return METTLE_FAILED();

    TCHAR file[_MAX_PATH];
    DWORD result = GetModuleFileName(nullptr, file, _countof(file));
    if (result == 0 || result >= _countof(file))
      return METTLE_FAILED();

    std::basic_ostringstream<TCHAR> args;
    args << file << TEXT(" --test-id ") << test.id << TEXT(" --log-fd ")
         << log_pipe.write_handle.handle();

    STARTUPINFO startup_info = { sizeof(STARTUPINFO) };
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    startup_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup_info.hStdOutput = stdout_pipe.write_handle;
    startup_info.hStdError = stderr_pipe.write_handle;

    PROCESS_INFORMATION proc_info;

    scoped_handle job;
    if(!(job = CreateJobObject(nullptr, nullptr)))
      return METTLE_FAILED();

    scoped_handle timeout_event;
    if(timeout_) {
      if(!(timeout_event = CreateWaitableTimer(nullptr, true, nullptr)))
        return METTLE_FAILED();
      LARGE_INTEGER t;
      // Convert from ms to 100s-of-nanoseconds (negative for relative time).
      t.QuadPart = -timeout_->count() * 10000;
      if(!SetWaitableTimer(timeout_event, &t, 0, nullptr, nullptr, false))
        return METTLE_FAILED();
    }

    if(!CreateProcess(
         nullptr, const_cast<PTCHAR>(args.str().c_str()), nullptr, nullptr, true,
         CREATE_SUSPENDED, nullptr, nullptr, &startup_info, &proc_info
       )) {
      return METTLE_FAILED();
    }
    scoped_handle subproc_handles[] = {proc_info.hProcess, proc_info.hThread};

    // Assign a job object to the child process (so we can kill the job later)
    // and then let it start running.
    if(!AssignProcessToJobObject(job, proc_info.hProcess))
      return METTLE_FAILED();
    if(!ResumeThread(proc_info.hThread))
      return METTLE_FAILED();

    if(!stdout_pipe.close_write() ||
       !stderr_pipe.close_write() ||
       !log_pipe.close_write())
      return METTLE_FAILED();

    std::string message;
    std::vector<readhandle> dests = {
      {stdout_pipe.read_handle, &output.stdout_log},
      {stderr_pipe.read_handle, &output.stderr_log},
      {log_pipe.read_handle,    &message}
    };
    std::vector<HANDLE> interrupts = {proc_info.hProcess};
    if(timeout_)
      interrupts.push_back(timeout_event);

    HANDLE finished = read_into(dests, INFINITE, interrupts);
    if(!finished)
      return METTLE_FAILED();
    // Do one last non-blocking read to get any data we might have missed.
    read_into(dests, 0, interrupts);

    // By now, the child process's main thread has returned, so kill any stray
    // processes in the job.
    TerminateJobObject(job, 1);

    if(finished == timeout_event) {
      std::ostringstream ss;
      ss << "Timed out after " << timeout_->count() << " ms";
      return { false, ss.str() };
    } else {
      DWORD exit_status;
      if(!GetExitCodeProcess(proc_info.hProcess, &exit_status))
        return METTLE_FAILED();
      return {exit_status == exit_code::success, message};
    }
  }

  const test_info *
  find_test(const suites_list &suites, test_uid id) {
    for(const auto &suite : suites) {
      for(const auto &test : suite.tests()) {
        if(test.id == id)
          return &test;
      }

      auto found = find_test(suite.subsuites(), id);
      if(found)
        return found;
    }

    return nullptr;
  }

  bool run_single_test(const test_info &test, HANDLE log_pipe) {
    auto result = test.function();

    DWORD size;
    if(!WriteFile(log_pipe, result.message.c_str(),
                  static_cast<DWORD>(result.message.size()), &size, nullptr)) {
      return false;
    }
    return result.passed;
  }

  int make_fd_private(HANDLE handle) {
    return SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0);
  }

} // namespace mettle
