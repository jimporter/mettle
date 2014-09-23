#ifndef INC_METTLE_CMD_PARSE_HPP
#define INC_METTLE_CMD_PARSE_HPP

#include <chrono>
#include <memory>
#include <vector>
#include <string>

#include <boost/any.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <mettle/log/core.hpp>
#include <mettle/log/indent.hpp>

#include "filters.hpp"
#include "optional.hpp"

namespace mettle {

struct generic_options {
  bool show_help = false;
};

boost::program_options::options_description
make_generic_options(generic_options &opts);

struct output_options {
  unsigned int verbosity = 1;
  bool color = false;
  size_t runs = 1;
  bool show_terminal = false;
  bool show_time = false;
};

boost::program_options::options_description
make_output_options(output_options &opts);

struct child_options {
  METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds> timeout;
  bool no_fork = false;
  filter_set filters;
};

boost::program_options::options_description
make_child_options(child_options &opts);

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

std::unique_ptr<log::file_logger>
make_progress_logger(indenting_ostream &out, const output_options &args);

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
