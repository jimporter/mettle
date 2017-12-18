#include "test_file.hpp"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>

#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include "glob.hpp"

namespace mettle {

test_file::test_file(std::string command) : command_(std::move(command)) {
#ifndef _WIN32
  auto args = boost::program_options::split_unix(command_);
#else
  auto args = boost::program_options::split_winmain(command_);
#endif

  for(auto &&arg : args) {
    if(arg.empty())
      continue;

    if(arg.find_first_of("?*[") != std::string::npos) {
      glob g(arg);
      std::copy(g.begin(), g.end(), std::back_inserter(args_));
    } else {
      args_.push_back(arg);
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
  } catch(...) {
    boost::throw_exception(invalid_option_value(val));
  }
}

} // namespace mettle
