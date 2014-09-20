#ifndef INC_METTLE_CMD_PARSE_HPP
#define INC_METTLE_CMD_PARSE_HPP

#include <memory>
#include <vector>
#include <string>

#include <boost/any.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <mettle/log/core.hpp>
#include <mettle/log/indent.hpp>

#include "filters.hpp"

namespace mettle {

std::unique_ptr<log::file_logger>
make_progress_logger(indenting_ostream &out, unsigned int verbosity,
                     size_t runs, bool show_terminal, bool fork_tests);

boost::program_options::option_description *
has_option(const boost::program_options::options_description &options,
           const boost::program_options::variables_map &args);

attr_filter parse_attr(const std::string &value);

void validate(boost::any &v, const std::vector<std::string> &values,
              attr_filter_set*, int);

void validate(boost::any &v, const std::vector<std::string> &values,
              name_filter_set*, int);

} // namespace mettle

#endif
