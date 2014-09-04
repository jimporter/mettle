#ifndef INC_METTLE_LOG_CORE_HPP
#define INC_METTLE_LOG_CORE_HPP

#include <sstream>
#include <string>
#include <vector>

namespace mettle {

namespace log {

  struct test_name {
    std::vector<std::string> suites;
    std::string test;
    size_t id;

    std::string full_name() const {
      std::stringstream s;
      for(const auto &i : suites)
        s << i << " > ";
      s << test;
      return s.str();
    }
  };

  inline bool operator ==(const test_name &lhs, const test_name &rhs) {
    return lhs.id == rhs.id;
  }
  inline bool operator !=(const test_name &lhs, const test_name &rhs) {
    return lhs.id != rhs.id;
  }
  inline bool operator <(const test_name &lhs, const test_name &rhs) {
    return lhs.id < rhs.id;
  }
  inline bool operator <=(const test_name &lhs, const test_name &rhs) {
    return lhs.id <= rhs.id;
  }
  inline bool operator >(const test_name &lhs, const test_name &rhs) {
    return lhs.id > rhs.id;
  }
  inline bool operator >=(const test_name &lhs, const test_name &rhs) {
    return lhs.id >= rhs.id;
  }

  struct test_output {
    std::string stdout, stderr;
  };

  class test_logger {
  public:
    virtual ~test_logger() {}

    virtual void started_run() = 0;
    virtual void ended_run() = 0;

    virtual void started_suite(const std::vector<std::string> &suites) = 0;
    virtual void ended_suite(const std::vector<std::string> &suites) = 0;

    virtual void started_test(const test_name &test) = 0;
    virtual void passed_test(const test_name &test, const test_output &log) = 0;
    virtual void failed_test(const test_name &test, const std::string &message,
                             const test_output &log) = 0;
    virtual void skipped_test(const test_name &test,
                              const std::string &message) = 0;
  };

  class file_logger : public test_logger {
  public:
    virtual ~file_logger() {}

    virtual void started_file(const std::string &file) = 0;
    virtual void ended_file(const std::string &file) = 0;
    virtual void failed_file(const std::string &file,
                             const std::string &message) = 0;
  };

}

} // namespace mettle

#endif
