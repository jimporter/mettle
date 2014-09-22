#include "cmd_line.hpp"

#include <regex>
#include <stdexcept>

#include <boost/program_options.hpp>

#include "log_quiet.hpp"
#include "log_verbose.hpp"

namespace mettle {

std::unique_ptr<log::file_logger>
make_progress_logger(indenting_ostream &out, unsigned int verbosity,
                     size_t runs, bool show_terminal, bool show_time,
                     bool fork_tests) {
  std::unique_ptr<log::file_logger> log;
  if(verbosity == 2) {
    if(!fork_tests) {
      if(show_terminal) {
        std::cerr << "--show-terminal requires forking tests" << std::endl;
        exit(1);
      }
    }
    log = std::make_unique<log::verbose>(out, runs, show_terminal, show_time);
  }
  else {
    if(show_terminal) {
      std::cerr << "--show-terminal requires verbosity of 2" << std::endl;
      exit(1);
    }
    if(show_time) {
      std::cerr << "--show-time requires verbosity of 2" << std::endl;
      exit(1);
    }

    if(verbosity == 1)
      log = std::make_unique<log::quiet>(out);
  }
  return log;
}

boost::program_options::option_description *
has_option(const boost::program_options::options_description &options,
           const boost::program_options::variables_map &args) {
  for(const auto &i : options.options()) {
    if(args.count(i->long_name()))
      return i.get();
  }
  return nullptr;
}

attr_filter parse_attr(const std::string &value) {
  enum parse_state {
    ITEM_START,
    NAME_START,
    NAME,
    VALUE_START,
    VALUE
  };

  attr_filter result;
  bool negated;
  parse_state state = ITEM_START;
  std::string::const_iterator i = value.begin(), start, name_start, name_end;
  do {
    switch(state) {
    case ITEM_START:
      if(i == value.end()) {
        throw std::invalid_argument("unexpected end of string");
      }
      else if(*i == '!') {
        negated = true;
        state = NAME_START;
        break;
      }
      negated = false;
    case NAME_START:
      if(i == value.end())
        throw std::invalid_argument("unexpected end of string");
      else if(*i == ',' || *i == '=')
        throw std::invalid_argument("expected attribute name");
      start = i;
      state = NAME;
      break;
    case NAME:
      if(i == value.end() || *i == ',') {
        auto item = has_attr(std::string(start, i));
        result.insert(negated ? !std::move(item) : std::move(item));
        state = ITEM_START;
      }
      else if(*i == '=') {
        name_start = start;
        name_end = i;
        state = VALUE_START;
      }
      break;
    case VALUE_START:
      start = i;
      state = VALUE;
    case VALUE:
      if(i == value.end() || *i == ',') {
        auto item = has_attr(
          std::string(name_start, name_end),
          std::string(start, i)
        );
        result.insert(negated ? !std::move(item) : std::move(item));
        state = ITEM_START;
      }
      break;
    default:
      assert(false && "invalid parse state");
    }
  } while(i++ != value.end());

  assert(state == ITEM_START);
  return result;
}

void validate(boost::any &v, const std::vector<std::string> &values,
              attr_filter_set*, int) {
  using namespace boost::program_options;
  if(v.empty())
    v = attr_filter_set();
  attr_filter_set* filters = boost::any_cast<attr_filter_set>(&v);
  assert(filters != nullptr);
  for(const auto &i : values) {
    try {
      filters->insert(parse_attr(i));
    }
    catch(...) {
      boost::throw_exception(invalid_option_value(i));
    }
  }
}

void validate(boost::any &v, const std::vector<std::string> &values,
              name_filter_set*, int) {
  using namespace boost::program_options;
  if(v.empty())
    v = name_filter_set();
  name_filter_set* filters = boost::any_cast<name_filter_set>(&v);
  assert(filters != nullptr);
  for(const auto &i : values) {
    try {
      filters->insert(std::regex(i));
    }
    catch(...) {
      boost::throw_exception(invalid_option_value(i));
    }
  }
}

} // namespace mettle
