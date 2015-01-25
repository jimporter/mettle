#ifndef INC_METTLE_SRC_TEST_FILE_HPP
#define INC_METTLE_SRC_TEST_FILE_HPP

#include <memory>
#include <string>
#include <vector>

#include <boost/any.hpp>

namespace mettle {

class test_file {
public:
  test_file(std::string command);

  const std::string & command() const {
    return command_;
  }

  operator const std::string &() const {
    return command_;
  }

  const std::vector<std::string> & args() const {
    return args_;
  }
private:
  std::string command_;
  std::vector<std::string> args_;
};

void validate(boost::any &v, const std::vector<std::string> &values,
              test_file*, int);

} // namespace mettle

#endif
