#ifndef INC_METTLE_DRIVER_LOG_XUNIT_HPP
#define INC_METTLE_DRIVER_LOG_XUNIT_HPP

#include <cstdint>
#include <memory>
#include <ostream>
#include <stack>

#include "core.hpp"
#include "indent.hpp"
#include "xml.hpp"
#include "../detail/export.hpp"

// Ignore warnings from MSVC about DLL interfaces.
#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(push)
#  pragma warning(disable:4251)
#endif

namespace mettle::log {

  class METTLE_PUBLIC xunit : public file_logger {
  public:
    xunit(std::string filename, std::size_t runs);
    // Exposed only for tests.
    xunit(std::unique_ptr<std::ostream> stream, std::size_t runs);

    void started_run() override;
    void ended_run() override;

    void started_suite(const std::vector<std::string> &suites) override;
    void ended_suite(const std::vector<std::string> &suites) override;

    void started_test(const test_name &test) override;
    void passed_test(const test_name &test, const test_output &output,
                     test_duration duration) override;
    void failed_test(const test_name &test, const std::string &message,
                     const test_output &output,
                     test_duration duration) override;
    void skipped_test(const test_name &test,
                      const std::string &message) override;

    void started_file(const std::string &file) override;
    void ended_file(const std::string &file) override;

    void failed_file(const std::string &file,
                     const std::string &message) override;
  private:
    struct suite_stack_item {
      suite_stack_item(std::string name)
        : elt{xml::element::make("testsuite")} {
        elt->attr("name", std::move(name));
      }

      suite_stack_item(suite_stack_item &&) = default;

      xml::element_ptr elt;
      std::size_t failures{0}, skips{0};
      test_duration duration{0};
    };

    suite_stack_item & current_suite();

    std::unique_ptr<std::ostream> out_;
    xml::document doc_;
    std::stack<suite_stack_item> suite_stack_;
    std::size_t tests_{0}, failures_{0}, skips_{0};
    test_duration duration_{0};
  };

} // namespace mettle::log

#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(pop)
#endif

#endif
