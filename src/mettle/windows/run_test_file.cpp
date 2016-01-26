#include "run_test_file.hpp"

#include <sstream>
#include <iomanip>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <mettle/driver/windows/scoped_pipe.hpp>

#include "../log_pipe.hpp"
#include "../../err_string.hpp"

namespace mettle {

namespace windows {

  namespace {
    inline file_result failed() {
      return {false, err_string(GetLastError())};
    }

    class quoted_arg {
    public:
      quoted_arg(const std::string &str) : str_(str) {}
    private:
      friend std::ostream &
      operator <<(std::ostream &os, const quoted_arg &arg) {
        if(arg.str_.find_first_of(" \"") == std::string::npos)
          return os << arg.str_;

        // Windows command line escaping is weird. It only treats '\' as an
        // escape if it's in a run of '\' immediately before a '"'.
        std::size_t num_backslashes = 0;
        os << '"';
        for(auto c : arg.str_) {
          switch(c) {
          case '\\':
            num_backslashes++;
            break;
          case '"':
            os << std::string(num_backslashes, '\\');
            num_backslashes = 0;
            break;
          default:
            num_backslashes = 0;
          }
          os << c;
        }
        return os << '"';
      }

      const std::string &str_;
    };

    std::string
    make_command(const std::vector<std::string> &argv) {
      std::ostringstream ss;
      ss << quoted_arg(argv[0]);
      for(std::size_t i = 1; i != argv.size(); i++)
        ss << " " << quoted_arg(argv[i]);
      return ss.str();
    }
  }

  file_result run_test_file(std::vector<std::string> args, log::pipe &logger) {
    scoped_pipe message_pipe;
    if(!message_pipe.open())
      return failed();
    if(!message_pipe.set_write_inherit(true))
      return failed();

    std::ostringstream ss;
    ss << message_pipe.write_handle.handle();
    args.insert(args.end(), { "--output-fd", ss.str() });
    std::string command = make_command(args);

    STARTUPINFOA startup_info = { sizeof(STARTUPINFOA) };
    PROCESS_INFORMATION proc_info;

    if(!CreateProcessA(
         args[0].c_str(), const_cast<char*>(command.c_str()), nullptr,
         nullptr, true, 0, nullptr, nullptr, &startup_info, &proc_info
       )) {
      return failed();
    }
    scoped_handle subproc_handles[] = {proc_info.hProcess, proc_info.hThread};

    if(!message_pipe.close_write()) {
      TerminateProcess(proc_info.hProcess, 1);
      return failed();
    }

    std::exception_ptr except;
    try {
      namespace io = boost::iostreams;
      io::stream<io::file_descriptor_source> fds(
        message_pipe.read_handle.handle(), io::never_close_handle
      );
      while(fds.peek() != EOF)
        logger(fds);
    }
    catch(...) {
      except = std::current_exception();
    }

    if(WaitForSingleObject(proc_info.hProcess, INFINITE)) {
      TerminateProcess(proc_info.hProcess, 1);
      return failed();
    }

    DWORD exit_code;
    if(!GetExitCodeProcess(proc_info.hProcess, &exit_code))
      return failed();

    if(exit_code) {
      std::ostringstream ssi;
      ssi << "Exited with status " << exit_code;
      return {false, ssi.str()};
    }
    else if(except) {
      try {
        std::rethrow_exception(except);
      }
      catch(const std::exception &e) {
        return {false, e.what()};
      }
    }
    else {
      return {true, ""};
    }
  }
}

} // namespace mettle
