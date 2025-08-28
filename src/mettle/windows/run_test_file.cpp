#include "run_test_file.hpp"

#include <sstream>
#include <iomanip>

// Ignore warnings about deprecated implicit copy constructor.
#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdeprecated"
#endif

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

#include <mettle/detail/source_location.hpp>
#include <mettle/driver/windows/scoped_pipe.hpp>

#include "../log_pipe.hpp"
#include "../../err_string.hpp"

#ifdef METTLE_NO_SOURCE_LOCATION
#  define METTLE_FAILED() failed(                                             \
     ::mettle::detail::source_location::current(__FILE__, __func__, __LINE__) \
   )
#else
#  define METTLE_FAILED() failed()
#endif

namespace mettle::windows {

  namespace {
    file_result failed(detail::source_location loc =
                         detail::source_location::current()) {
      std::ostringstream ss;
      ss << "Fatal error at " << loc.file_name() << ":" << loc.line() << "\n"
         << err_string(GetLastError());
      return {false, ss.str()};
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

    template<typename T>
    struct arg_converter {
      static std::basic_string<T> convert(std::string_view arg) {
        return arg;
      }
    };

    template<>
    struct arg_converter<wchar_t> {
      static std::wstring convert(std::string_view arg) {
        std::wstring result(arg.size(), L'\0');
        int len = MultiByteToWideChar(CP_UTF8, 0, arg.data(), arg.size(),
                                      result.data(), result.size());
        result.resize(len);
        return result;
      }
    };

    std::basic_string<TCHAR>
    make_command(const std::vector<std::string> &argv) {
      std::ostringstream ss;
      ss << quoted_arg(argv[0]);
      for(std::size_t i = 1; i != argv.size(); i++)
        ss << " " << quoted_arg(argv[i]);
      return arg_converter<TCHAR>::convert(ss.str());
    }
  }

  file_result run_test_file(std::vector<std::string> args, log::pipe &logger) {
    scoped_pipe message_pipe;
    if(!message_pipe.open())
      return METTLE_FAILED();
    if(!message_pipe.set_write_inherit(true))
      return METTLE_FAILED();

    std::ostringstream ss;
    ss << message_pipe.write_handle.handle();
    args.insert(args.end(), { "--output-fd", ss.str() });
    std::basic_string<TCHAR> command = make_command(args);

    STARTUPINFO startup_info = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION proc_info;

    if(!CreateProcess(
         nullptr, const_cast<PTCHAR>(command.c_str()), nullptr,
         nullptr, true, 0, nullptr, nullptr, &startup_info, &proc_info
       )) {
      return METTLE_FAILED();
    }
    scoped_handle subproc_handles[] = {proc_info.hProcess, proc_info.hThread};

    if(!message_pipe.close_write()) {
      TerminateProcess(proc_info.hProcess, 1);
      return METTLE_FAILED();
    }

    std::exception_ptr except;
    try {
      namespace io = boost::iostreams;
      io::stream<io::file_descriptor_source> fds(
        message_pipe.read_handle.handle(), io::never_close_handle
      );
      while(fds.peek() != EOF)
        logger(fds);
    } catch(...) {
      except = std::current_exception();
    }

    if(WaitForSingleObject(proc_info.hProcess, INFINITE)) {
      TerminateProcess(proc_info.hProcess, 1);
      return METTLE_FAILED();
    }

    DWORD exit_code;
    if(!GetExitCodeProcess(proc_info.hProcess, &exit_code))
      return METTLE_FAILED();

    if(exit_code) {
      std::ostringstream ssi;
      ssi << "Exited with status " << exit_code;
      return {false, ssi.str()};
    } else if(except) {
      try {
        std::rethrow_exception(except);
      } catch(const std::exception &e) {
        return {false, e.what()};
      }
    } else {
      return {true, ""};
    }
  }

} // namespace mettle::windows
