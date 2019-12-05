#include <mettle/driver/cmd_line.hpp>

#include <cassert>
#include <cstdint>
#include <regex>
#include <sstream>
#include <stdexcept>

#include <boost/program_options.hpp>

#include <mettle/driver/log/counter.hpp>
#include <mettle/driver/log/brief.hpp>
#include <mettle/driver/log/verbose.hpp>
#include <mettle/driver/log/xunit.hpp>
#include <mettle/detail/algorithm.hpp>

#ifndef _WIN32
#  include <unistd.h>
#else
#  include <io.h>
#endif

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
  make_driver_options(driver_options &opts) {
    using namespace boost::program_options;
    options_description desc("Driver options");
    desc.add_options()
      ("timeout,t", value(&opts.timeout)->value_name("TIME"), "timeout in ms")
      ("test,T", value(&opts.filters.by_name)->value_name("REGEX"),
       "regex matching names of tests to run")
      ("attr,a", value(&opts.filters.by_attr)->value_name("ATTR"),
       "attributes of tests to run")
    ;
    return desc;
  }

// MSVC doesn't understand [[noreturn]], so just ignore the warning here.
#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(push)
#  pragma warning(disable:4715)
#endif

  bool color_enabled(color_option opt, int fd) {
    switch(opt) {
    case color_option::never:
      return false;
    case color_option::automatic:
#ifndef _WIN32
      return isatty(fd);
#else
      return _isatty(fd);
#endif
    case color_option::always:
      return true;
    default:
      assert(false && "unexpected value");
    }
  }

#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(pop)
#endif

  boost::program_options::options_description
  make_output_options(output_options &opts, const logger_factory &factory) {
    using namespace boost::program_options;

    std::ostringstream ss;
    ss << "set output format (one of: " << detail::joined(
      factory, [](auto &&i) { return i.first; }
    ) << "; default: " << opts.output << ")";

    options_description desc("Output options");
    desc.add_options()
      ("output,o", value(&opts.output)->value_name("FORMAT"), ss.str().c_str())
      ("color", value(&opts.color)->value_name("WHEN"),
       "show colored output (one of: never, auto, always; default: auto)")
      (",c", value(&opts.color)->zero_tokens()
            ->implicit_value(color_option::always, "always"),
       "show colored output (equivalent to `--color=always`)")
      ("runs,n", value(&opts.runs)->value_name("N"), "number of test runs")
      ("show-terminal", value(&opts.show_terminal)->zero_tokens(),
       "show terminal output for each test")
      ("show-time", value(&opts.show_time)->zero_tokens(),
       "show the duration for each test")
      ("file,f", value(&opts.file)->value_name("FILE"),
       ("file to print test results to (for xunit only; default: " + opts.file +
        ")").c_str())
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

  logger_factory make_logger_factory() {
    logger_factory f;

    f.add("silent", [](indenting_ostream &, const output_options &) {
      return std::unique_ptr<log::file_logger>();
    });
    f.add("counter", [](indenting_ostream &out, const output_options &) {
      return std::make_unique<log::counter>(out);
    });
    f.add("brief", [](indenting_ostream &out, const output_options &) {
      return std::make_unique<log::brief>(out);
    });
    f.add("verbose", [](indenting_ostream &out, const output_options &args) {
      return std::make_unique<log::verbose>(
        out, args.runs, args.show_time, args.show_terminal
      );
    });
    f.add("xunit", [](indenting_ostream &, const output_options &args) {
      return std::make_unique<log::xunit>(args.file, args.runs);
    });

    return f;
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
        } else if(*i == '!') {
          negated = true;
          state = NAME_START;
          break;
        }
        negated = false;
        [[fallthrough]];
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
        } else if(*i == '=') {
          name_start = start;
          name_end = i;
          state = VALUE_START;
        }
        break;
      case VALUE_START:
        start = i;
        state = VALUE;
        [[fallthrough]];
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
                color_option*, int) {
    using namespace boost::program_options;
    validators::check_first_occurrence(v);
    const std::string &val = validators::get_single_string(values);

    if(val == "never")
      v = color_option::never;
    else if(val == "auto")
      v = color_option::automatic;
    else if(val == "always")
      v = color_option::always;
    else
      boost::throw_exception(invalid_option_value(val));
  }

  void validate(boost::any &v, const std::vector<std::string> &values,
                attr_filter_set*, int) {
    using namespace boost::program_options;
    if(v.empty())
      v = attr_filter_set();
    attr_filter_set *filters = boost::any_cast<attr_filter_set>(&v);
    assert(filters != nullptr);
    for(const auto &i : values) {
      try {
        filters->insert(parse_attr(i));
      } catch(...) {
        boost::throw_exception(invalid_option_value(i));
      }
    }
  }

  void validate(boost::any &v, const std::vector<std::string> &values,
                name_filter_set*, int) {
    using namespace boost::program_options;
    if(v.empty())
      v = name_filter_set();
    name_filter_set *filters = boost::any_cast<name_filter_set>(&v);
    assert(filters != nullptr);
    for(const auto &i : values) {
      try {
        filters->insert(std::regex(i));
      } catch(...) {
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
      v = std::chrono::milliseconds(boost::lexical_cast<std::size_t>(val));
    } catch(...) {
      boost::throw_exception(invalid_option_value(val));
    }
  }

#ifdef _WIN32
  void validate(boost::any &v, const std::vector<std::string> &values,
                HANDLE*, int) {
    using namespace boost::program_options;
    validators::check_first_occurrence(v);
    const std::string &val = validators::get_single_string(values);

    try {
      HANDLE h;
      std::istringstream is(val);
      is >> h;
      v = h;
    } catch (...) {
      boost::throw_exception(invalid_option_value(val));
    }
  }
#endif

} // namespace boost
