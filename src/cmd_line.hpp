#ifndef INC_METTLE_SRC_CMD_LINE_HPP
#define INC_METTLE_SRC_CMD_LINE_HPP

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

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

inline boost::program_options::option_description *
has_option(const boost::program_options::options_description &options,
           const boost::program_options::variables_map &args) {
  for(const auto &i : options.options()) {
    if(args.count(i->long_name()))
      return i.get();
  }
  return nullptr;
}

} // namespace mettle

#endif
