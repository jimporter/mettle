#include <mettle/driver/cmd_line.hpp>

#include <regex>
#include <stdexcept>

#include <boost/program_options.hpp>

#include <mettle/driver/log/quiet.hpp>
#include <mettle/driver/log/verbose.hpp>

namespace mettle {

boost::program_options::options_description
make_generic_options(generic_options &opts) {
  using namespace boost::program_options;
  options_description desc("Generic options");
  desc.add_options()
    ("help,h", value(&opts.show_help)->zero_tokens(), "show help")
  ;
  return desc;
}

boost::program_options::options_description
make_output_options(output_options &opts) {
  using namespace boost::program_options;
  options_description desc("Output options");
  desc.add_options()
    ("verbose,v", value(&opts.verbosity)->implicit_value(2),
     "show verbose output")
    ("color,c", value(&opts.color)->zero_tokens(), "show colored output")
    ("runs,n", value(&opts.runs), "number of test runs")
    ("show-terminal", value(&opts.show_terminal)->zero_tokens(),
     "show terminal output for each test")
    ("show-time", value(&opts.show_time)->zero_tokens(),
     "show the duration for each test")
  ;
  return desc;
}

boost::program_options::options_description
make_child_options(child_options &opts) {
  using namespace boost::program_options;
  options_description desc("Child options");
  desc.add_options()
    ("timeout,t", value(&opts.timeout), "timeout in ms")
    ("no-fork", value(&opts.no_fork)->zero_tokens(),
     "don't fork for each test")
    ("test,T", value(&opts.filters.by_name),
     "regex matching names of tests to run")
    ("attr,a", value(&opts.filters.by_attr), "attributes of tests to run")
  ;
  return desc;
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

std::unique_ptr<log::file_logger>
make_progress_logger(indenting_ostream &out, const output_options &args) {
  switch(args.verbosity) {
  case 1:
    return std::make_unique<log::quiet>(out);
  case 2:
    return std::make_unique<log::verbose>(out, args.runs, args.show_time,
                                          args.show_terminal);
  default:
    return {};
  }
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

namespace boost {

void validate(boost::any &v, const std::vector<std::string> &values,
              std::chrono::milliseconds*, int) {
  using namespace boost::program_options;
  validators::check_first_occurrence(v);
  const std::string &val = validators::get_single_string(values);

  try {
    v = std::chrono::milliseconds(boost::lexical_cast<size_t>(val));
  }
  catch(...) {
    boost::throw_exception(invalid_option_value(val));
  }
}

} // namespace boost
