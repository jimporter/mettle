#include "parse_attr.hpp"

#include <boost/program_options.hpp>

namespace mettle {

namespace detail {
  // XXX: These could use some bullet-proofing.
  attr_filter::filter_item parse_item(const std::string &value) {
    bool negate = !value.empty() && value[0] == '!';
    size_t i = value.find('=');

    attr_filter::filter_item filter;
    if(i == std::string::npos)
      filter = has_attr(value.substr(negate ? 1 : 0));
    else
      filter = has_attr(value.substr(negate ? 1 : 0, i), value.substr(i + 1));

    return negate ? !std::move(filter) : filter;
  }

  attr_filter parse_attr(const std::string &value) {
    attr_filter filter;
    size_t start = 0, end;
    while(( end = value.find(',', start) ) != std::string::npos) {
      auto item = value.substr(start, end);
      start = end + 1;
      filter.insert(parse_item(item));
    }
    filter.insert(parse_item(value.substr(start)));
    return std::move(filter);
  }
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
      filters->insert(detail::parse_attr(i));
    }
    catch(...) {
      boost::throw_exception(invalid_option_value(i));
    }
  }
}

} // namespace mettle
