#include <mettle/driver/log/xunit.hpp>

#include <fstream>
#include <stdexcept>

#include <mettle/detail/algorithm.hpp>

namespace mettle::log {

  static inline std::string get_duration(test_duration duration) {
    using seconds = std::chrono::duration<
      double, std::chrono::seconds::period
    >;
    return std::to_string(seconds(duration).count());
  }

  static xml::element_ptr
  message_element(std::string name, std::string message) {
    auto e = xml::element::make(std::move(name));
    e->attr("message", std::move(message));
    return e;
  }

  static xml::element_ptr test_element(const test_name &test) {
    auto e = xml::element::make("testcase");
    e->attr("name", test.name);
    return e;
  }

  static void append_test_output(xml::element_ptr &test,
                                 const test_output &output) {
    if(!output.stdout_log.empty()) {
      auto sysout = xml::element::make("system-out");
      sysout->append_child(xml::text::make(output.stdout_log));
      test->append_child(std::move(sysout));
    }
    if(!output.stderr_log.empty()) {
      auto syserr = xml::element::make("system-err");
      syserr->append_child(xml::text::make(output.stderr_log));
      test->append_child(std::move(syserr));
    }
  }


  xunit::xunit(std::string filename, std::size_t runs)
    : xunit(std::make_unique<std::ofstream>(std::move(filename)), runs) {}

  xunit::xunit(std::unique_ptr<std::ostream> stream, std::size_t runs)
    : out_(std::move(stream)), doc_("testsuites") {
    if(runs != 1)
      throw std::domain_error("xunit logger may only be used with --runs=1");
  }

  void xunit::started_run() {}

  void xunit::ended_run() {
    doc_.root()->attr("tests", std::to_string(tests_));
    doc_.root()->attr("failures", std::to_string(failures_));
    doc_.root()->attr("skipped", std::to_string(skips_));
    doc_.root()->attr("time", get_duration(duration_));

    doc_.write(*out_);
  }

  void xunit::started_suite(const std::vector<std::string> &suites) {
    using namespace mettle::detail;
    auto name = stringify(joined(suites, std::identity{}, " > "));
    suite_stack_.emplace(std::move(name));
  }

  void xunit::ended_suite(const std::vector<std::string> &) {
    auto &suite = current_suite();
    if(suite.elt->children_size()) {
      suite.elt->attr("tests", std::to_string(suite.elt->children_size()));
      suite.elt->attr("failures", std::to_string(suite.failures));
      suite.elt->attr("skipped", std::to_string(suite.skips));
      suite.elt->attr("time", get_duration(suite.duration));
      doc_.root()->append_child(std::move(suite.elt));
    }
    suite_stack_.pop();
  }

  void xunit::started_test(const test_name &) {}

  void xunit::passed_test(const test_name &test, const test_output &output,
                          test_duration duration) {
    auto &suite = current_suite();
    auto t = test_element(test);
    t->attr("time", get_duration(duration));
    append_test_output(t, output);
    suite.elt->append_child(std::move(t));
    tests_++;
    suite.duration += duration;
    duration_ += duration;
  }

  void xunit::failed_test(const test_name &test, const std::string &message,
                          const test_output &output, test_duration duration) {
    auto &suite = current_suite();
    auto t = test_element(test);
    t->attr("time", get_duration(duration));
    t->append_child(message_element("failure", message));
    append_test_output(t, output);
    suite.elt->append_child(std::move(t));

    tests_++;
    suite.failures++;
    failures_++;
    suite.duration += duration;
    duration_ += duration;
  }

  void xunit::skipped_test(const test_name &test, const std::string &message) {
    auto &suite = current_suite();
    auto t = test_element(test);
    t->append_child(message_element("skipped", message));
    suite.elt->append_child(std::move(t));

    tests_++;
    suite.skips++;
    skips_++;
  }

  void xunit::started_file(const test_file &) {}
  void xunit::ended_file(const test_file &) {}

  void xunit::failed_file(const test_file &file, const std::string &message) {
    auto suite = xml::element::make("testsuite");
    suite->attr("name", "file `" + file.name + "`");
    suite->attr("tests", "1");
    suite->attr("failures", "1");
    suite->attr("time", "0");

    auto t = xml::element::make("testcase");
    t->attr("name", "<file>");
    t->attr("time", "0");
    t->append_child(message_element("failure", message));
    suite->append_child(std::move(t));

    doc_.root()->append_child(std::move(suite));

    tests_++;
    failures_++;
  }

  xunit::suite_stack_item & xunit::current_suite() {
    if(suite_stack_.empty())
      std::abort();
    return suite_stack_.top();
  }

} // namespace mettle::log
