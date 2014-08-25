#ifndef INC_METTLE_PARSE_ATTR_HPP
#define INC_METTLE_PARSE_ATTR_HPP

#include <vector>
#include <string>

#include <boost/any.hpp>

#include <mettle/attributes.hpp>

namespace mettle {

void validate(boost::any &v, const std::vector<std::string> &values,
              attr_filter_set*, int);

} // namespace mettle

#endif
