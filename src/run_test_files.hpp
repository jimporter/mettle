#ifndef INC_METTLE_SRC_RUN_TEST_FILES_HPP
#define INC_METTLE_SRC_RUN_TEST_FILES_HPP

#include <boost/any.hpp>

#include <mettle/driver/log/core.hpp>

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

void run_test_files(
  const std::vector<test_file> &files, log::file_logger &logger,
  const std::vector<std::string> &args = std::vector<std::string>{}
);

} // namespace mettle

#endif
