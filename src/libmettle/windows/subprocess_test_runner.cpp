#include "subprocess_test_runner.hpp"

#include <iostream>
#include <memory>
#include <sstream>

#include <windows.h>

#include <mettle/driver/windows/scoped_pipe.hpp>
#include <mettle/driver/windows/subprocess.hpp>

namespace mettle {

namespace windows {

  namespace {
    test_result failed() {
      char *tmp;
      FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        reinterpret_cast<char*>(&tmp), 0, nullptr
      );
      std::unique_ptr<char, HLOCAL (__stdcall*)(HLOCAL)> msg(tmp, LocalFree);
      return { false, msg.get() };
    }
  }

  test_result subprocess_test_runner::operator ()(
    const test_info &test, log::test_output &output
  ) const {
    scoped_pipe stdout_pipe, stderr_pipe, log_pipe;
    if(!stdout_pipe.open({true, false}) ||
       !stderr_pipe.open({true, false}) ||
       !log_pipe.open({true, false}))
      return failed();

    if(!stdout_pipe.set_write_inherit(true) ||
       !stderr_pipe.set_write_inherit(true) ||
       !log_pipe.set_write_inherit(true))
      return failed();

    char file[_MAX_PATH];
    if(GetModuleFileNameA(nullptr, file, sizeof(file)) == sizeof(file))
      return failed();

    std::ostringstream args;
    args << file << " --test-id " << test.id << " --log-fd "
         << log_pipe.write_handle.handle();

    STARTUPINFOA startup_info = { sizeof(STARTUPINFOA) };
    memset(&startup_info, 0, sizeof(STARTUPINFO));
    startup_info.dwFlags = STARTF_USESTDHANDLES;
    startup_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup_info.hStdOutput = stdout_pipe.write_handle;
    startup_info.hStdError = stderr_pipe.write_handle;

    PROCESS_INFORMATION proc_info;

    scoped_handle job;
    if(!(job = CreateJobObject(nullptr, nullptr)))
      return failed();

    scoped_handle timeout_event;
    if(timeout_) {
      if(!(timeout_event = CreateWaitableTimer(nullptr, true, nullptr)))
        return failed();
      LARGE_INTEGER t;
      // Convert from ms to 100s-of-nanoseconds (negative for relative time).
      t.QuadPart = -timeout_->count() * 10000;
      if(!SetWaitableTimer(timeout_event, &t, 0, nullptr, nullptr, false))
        return failed();
    }

    if(!CreateProcessA(
         file, const_cast<char*>(args.str().c_str()), nullptr, nullptr, true,
         CREATE_SUSPENDED, nullptr, nullptr, &startup_info, &proc_info
       )) {
      return failed();
    }
    scoped_handle subproc_handles[] = {proc_info.hProcess, proc_info.hThread};

    // Assign a job object to the child process (so we can kill the job later)
    // and then let it start running.
    if(!AssignProcessToJobObject(job, proc_info.hProcess))
      return failed();
    if(!ResumeThread(proc_info.hThread))
      return failed();

    if(!stdout_pipe.close_write() ||
       !stderr_pipe.close_write() ||
       !log_pipe.close_write())
      return failed();

    std::string message;
    std::vector<readhandle> dests = {
      {stdout_pipe.read_handle, &output.stdout_log},
      {stderr_pipe.read_handle, &output.stderr_log},
      {log_pipe.read_handle,    &message}
    };
    std::vector<HANDLE> interrupts = {proc_info.hProcess};
    if(timeout_)
      interrupts.push_back(timeout_event);

    HANDLE finished = read_into(dests, interrupts);
    if(!finished)
      return failed();

    // By now, the child process's main thread has returned, so kill any stray
    // processes in the job.
    TerminateJobObject(job, 1);

    if(finished == timeout_event) {
      std::ostringstream ss;
      ss << "Timed out after " << timeout_->count() << " ms";
      return { false, ss.str() };
    }
    else {
      DWORD exit_code;
      if(!GetExitCodeProcess(proc_info.hProcess, &exit_code))
        return failed();
      return {exit_code == 0, message};
    }
  }

  const test_info *
  find_test(const suites_list &suites, test_uid id) {
    for(const auto &suite : suites) {
      for(const auto &test : suite) {
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
    if(!WriteFile(log_pipe, result.message.c_str(), result.message.size(),
                  &size, nullptr)) {
      return false;
    }
    return result.passed;
  }

}

} // namespace mettle
