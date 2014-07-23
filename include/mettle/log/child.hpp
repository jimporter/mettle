#ifndef INC_METTLE_LOG_CHILD_HPP
#define INC_METTLE_LOG_CHILD_HPP

#include <ostream>
#include <iomanip>

#include "term.hpp"
#include "core.hpp"

namespace mettle {

namespace log {

  // This code is absolutely horrible for a number of reasons, but it gets the
  // job done. Things to fix: use an actual, language-agnostic message format
  // (bencode?) instead of just using std::quoted to pass strings along; also,
  // forward the file descriptors for each test's stdout and stderr on to the
  // parent process so that we don't make a bunch of totally-unnecessary copies
  // of the output.
  //
  // See also <include/mettle/file_runner.hpp>.
  class child : public test_logger {
  public:
    child(std::ostream &out) : out(out) {}

    void start_run() {
      out << "start_run" << std::endl;
    }
    void end_run() {
      out << "end_run" << std::endl;
    }

    void start_suite(const std::vector<std::string> &suites) {
      out << "start_suite ";
      write_suites(suites);
      out << std::endl;
    }
    void end_suite(const std::vector<std::string> &suites) {
      out << "end_suite ";
      write_suites(suites);
      out << std::endl;
    }

    void start_test(const test_name &test) {
      out << "start_test ";
      write_test_name(test);
      out << std::endl;
    }

    void passed_test(const test_name &test, test_output &output) {
      out << "passed_test ";
      write_test_name(test);
      out << " ";
      write_test_output(output);
      out << std::endl;
    }
    void failed_test(const test_name &test, const std::string &message,
                     test_output &output) {
      out << "failed_test ";
      write_test_name(test);
      out << " " << std::quoted(message) << " ";
      write_test_output(output);
      out << std::endl;
    }
    void skipped_test(const test_name &test) {
      out << "skipped_test ";
      write_test_name(test);
      out << std::endl;
    }
  private:
    void write_suites(const std::vector<std::string> &suites) {
      out << suites.size();
      for(auto &&i : suites)
        out << " " << std::quoted(i);
    }

    void write_test_name(const test_name &test) {
      write_suites(test.suites);
      out << std::quoted(test.test) << " " << test.id;
    }

    void write_test_output(const test_output &output) {
      out << std::quoted(output.stdout.str()) << " "
                << std::quoted(output.stderr.str());
    }

    std::ostream &out;
  };

}

} // namespace mettle

#endif
