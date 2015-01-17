#ifndef INC_METTLE_DRIVER_CMD_LINE_HPP
#define INC_METTLE_DRIVER_CMD_LINE_HPP

#include <chrono>
#include <memory>
#include <vector>
#include <string>

#include <boost/any.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include "filters.hpp"
#include "object_factory.hpp"
#include "detail/optional.hpp"
#include "log/core.hpp"
#include "log/indent.hpp"

namespace mettle {

struct generic_options {
  bool show_help = false;
};

boost::program_options::options_description
make_generic_options(generic_options &opts);

struct driver_options {
  METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds> timeout;
  filter_set filters;
};

boost::program_options::options_description
make_driver_options(driver_options &opts);

struct output_options {
  std::string output;
  bool color = false;
  size_t runs = 1;
  bool show_terminal = false;
  bool show_time = false;
};

using logger_factory = object_factory<
  log::file_logger, indenting_ostream &, const output_options &
>;
logger_factory make_logger_factory();

boost::program_options::options_description
make_output_options(output_options &opts, const logger_factory &factory);

boost::program_options::option_description *
has_option(const boost::program_options::options_description &options,
           const boost::program_options::variables_map &args);

template<typename Char>
std::vector<std::basic_string<Char>> filter_options(
  const boost::program_options::basic_parsed_options<Char> &parsed,
  const boost::program_options::options_description &desc
) {
  std::vector<std::basic_string<Char>> filtered;
  for(auto &&option : parsed.options) {
    if(desc.find_nothrow(option.string_key, false)) {
      auto &&tokens = option.original_tokens;
      filtered.insert(filtered.end(), tokens.begin(), tokens.end());
    }
  }
  return filtered;
}

attr_filter parse_attr(const std::string &value);

void validate(boost::any &v, const std::vector<std::string> &values,
              attr_filter_set*, int);

void validate(boost::any &v, const std::vector<std::string> &values,
              name_filter_set*, int);

} // namespace mettle

// Put these in the boost namespace so that ADL picks them up (via the
// boost::any parameter).
namespace boost {

void validate(boost::any &v, const std::vector<std::string> &values,
              std::chrono::milliseconds*, int);

template<typename T>
void validate(boost::any &v, const std::vector<std::string> &values,
              METTLE_OPTIONAL_NS::optional<T>*, int) {
  using namespace boost::program_options;
  using optional_t = METTLE_OPTIONAL_NS::optional<T>;

  if(v.empty())
    v = optional_t();
  auto *val = boost::any_cast<optional_t>(&v);
  assert(val);

  boost::any a;
  validate(a, values, static_cast<T*>(nullptr), 0);
  *val = boost::any_cast<T>(a);
}

} // namespace boost


#endif
