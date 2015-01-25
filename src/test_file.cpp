#include "test_file.hpp"

#include <cstdint>
#include <stdexcept>

#include <glob.h>

#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

namespace mettle {

test_file::test_file(std::string command) : command_(std::move(command)) {
  using separator = boost::escaped_list_separator<char>;
  boost::tokenizer<separator> tok(
    command_.begin(), command_.end(), separator("\\", " \t", "'\"")
  );

  for(auto &&token : tok) {
    if(token.empty())
      continue;

    if(token.find_first_of("?*[") != std::string::npos) {
      glob_t g;
      if(glob(token.c_str(), 0, nullptr, &g) != 0)
        throw std::runtime_error("invalid glob \"" + token + "\"");
      for(std::size_t i = 0; i != g.gl_pathc; i++)
        args_.push_back(g.gl_pathv[i]);
      globfree(&g);
    }
    else {
      args_.push_back(token);
    }
  }
}

void validate(boost::any &v, const std::vector<std::string> &values,
              test_file*, int) {
  using namespace boost::program_options;
  validators::check_first_occurrence(v);
  const std::string &val = validators::get_single_string(values);

  try {
    v = test_file(val);
  }
  catch(...) {
    boost::throw_exception(invalid_option_value(val));
  }
}

} // namespace mettle
