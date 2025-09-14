#ifndef INC_METTLE_SUITE_COMPILED_SUITE_HPP
#define INC_METTLE_SUITE_COMPILED_SUITE_HPP

#include <functional>
#include <string>
#include <vector>

#include "attributes.hpp"
#include "../test_uid.hpp"
#include "../detail/forward_like.hpp"
#include "../detail/source_location.hpp"

namespace mettle {

  struct test_result {
    bool passed;
    std::string message;
  };

  template<typename Function>
  struct basic_test_info {
    using function_type = std::function<Function>;

    basic_test_info(
      std::string name, function_type function, attributes attrs,
      detail::source_location loc
    ) : id(detail::make_test_uid()), name(std::move(name)),
        function(std::move(function)), attrs(std::move(attrs)),
        location(std::move(loc)) {}

    test_uid id;
    std::string name;
    function_type function;
    attributes attrs;
    detail::source_location location;
  };

  template<typename Function>
  class compiled_suite {
    template<typename>
    friend class compiled_suite;
  public:
    using test_info = basic_test_info<Function>;
    using iterator = typename std::vector<test_info>::const_iterator;

    template<typename String, typename Tests, typename Subsuites,
             typename Compile>
    compiled_suite(
      String &&name, Tests &&tests, Subsuites &&subsuites,
      const attributes &attrs, Compile &&compile
    ) : name_(std::forward<String>(name)) {
      for(auto &&test : tests) {
        tests_.emplace_back(
          detail::forward_like<Tests>(test.name),
          compile(detail::forward_like<Tests>(test.function)),
          unite(detail::forward_like<Tests>(test.attrs), attrs),
          detail::forward_like<Tests>(test.location)
        );
      }
      for(auto &&ss : subsuites) {
        subsuites_.emplace_back(
          detail::forward_like<Subsuites>(ss), attrs, compile
        );
      }
    }

    template<typename Function2, typename Compile>
    compiled_suite(const compiled_suite<Function2> &suite,
                   const attributes &attrs, Compile &&compile)
      : compiled_suite(suite.name_, suite.tests_, suite.subsuites_, attrs,
                       std::forward<Compile>(compile)) {}

    template<typename Function2, typename Compile>
    compiled_suite(compiled_suite<Function2> &&suite,
                   const attributes &attrs, Compile &&compile)
      : compiled_suite(std::move(suite.name_), std::move(suite.tests_),
                       std::move(suite.subsuites_), attrs,
                       std::forward<Compile>(compile)) {}

    const std::string & name() const {
      return name_;
    }

    const std::vector<test_info> & tests() const {
      return tests_;
    }

    const std::vector<compiled_suite> & subsuites() const {
      return subsuites_;
    }
  private:
    std::string name_;
    std::vector<test_info> tests_;
    std::vector<compiled_suite> subsuites_;
  };

  using runnable_suite = compiled_suite<test_result()>;
  using suites_list = std::vector<runnable_suite>;
  using test_info = runnable_suite::test_info;

} // namespace mettle

#endif
