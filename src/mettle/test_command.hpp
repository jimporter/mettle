#ifndef INC_METTLE_SRC_METTLE_TEST_COMMAND_HPP
#define INC_METTLE_SRC_METTLE_TEST_COMMAND_HPP

#include <memory>
#include <string>
#include <vector>

#include <boost/any.hpp>

namespace mettle {

  class test_command {
  public:
    test_command(std::string command);

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
                test_command*, int);

} // namespace mettle

#endif
