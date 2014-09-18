#ifndef INC_METTLE_SRC_CMD_LINE_HPP
#define INC_METTLE_SRC_CMD_LINE_HPP

#include <mettle/log/quiet.hpp>
#include <mettle/log/verbose.hpp>

namespace mettle {

inline std::unique_ptr<log::file_logger>
make_progress_logger(indenting_ostream &out, unsigned int verbosity,
                     size_t runs, bool show_terminal, bool fork_tests) {
  std::unique_ptr<log::file_logger> log;
  if(verbosity == 2) {
    if(!fork_tests) {
      if(show_terminal) {
        std::cerr << "--show-terminal requires forking tests" << std::endl;
        exit(1);
      }
    }
    log = std::make_unique<log::verbose>(out, runs, show_terminal);
  }
  else {
    if(show_terminal) {
      std::cerr << "--show-terminal requires verbosity of 2" << std::endl;
      exit(1);
    }

    if(verbosity == 1)
      log = std::make_unique<log::quiet>(out);
  }
  return log;
}

} // namespace mettle

#endif
