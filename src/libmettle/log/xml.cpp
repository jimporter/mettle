#include <mettle/driver/log/xml.hpp>

#include <map>
#include <regex>

#include <mettle/detail/string_algorithm.hpp>

static std::map<char, std::string> xml_replace = {
  {'<', "&lt;"},
  {'>', "&gt;"},
  {'&', "&amp;"},
  {'"', "&quot;"}
};

// Match anything that 1) starts with "xml" or anything other than a letter or
// underscore, or 2) contains anything other than alphanumerics, underscores,
// hyphens, and dots.
static std::regex invalid_name_re(
  "(^xml|^[^A-Za-z_]|[^A-Za-z0-9_.\\-])",
  std::regex::icase
);

namespace mettle {

namespace log {

  bool xml::valid_name(const std::string &s) {
    return !s.empty() && !std::regex_search(s, invalid_name_re);
  }

  void xml::element::write(indenting_ostream &out) const {
    out << "<" << tag_;
    for(auto &&i : attrs_)
      out << " " << i.first << "=\"" << detail::escaped(i.second, xml_replace)
          << "\"";
    if(children_.empty()) {
      out << "/>\n";
      return;
    }

    out << ">\n";
    {
      scoped_indent si(out);
      for(auto &&i : children_)
        i->write(out);
    }
    out << "</" << tag_ << ">\n";
  }

  void xml::text::write(indenting_ostream &out) const {
    out << detail::escaped(text_content_, xml_replace) << "\n";
  }

}

} // namespace mettle
