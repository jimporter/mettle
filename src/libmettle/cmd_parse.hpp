#ifndef INC_METTLE_CMD_PARSE_HPP
#define INC_METTLE_CMD_PARSE_HPP

#include <vector>
#include <string>

#include <boost/any.hpp>

#include "filters.hpp"

namespace mettle {

attr_filter parse_attr(const std::string &value);

void validate(boost::any &v, const std::vector<std::string> &values,
              attr_filter_set*, int);

void validate(boost::any &v, const std::vector<std::string> &values,
              name_filter_set*, int);

} // namespace mettle

#endif
