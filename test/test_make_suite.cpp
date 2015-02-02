#include <mettle.hpp>
using namespace mettle;

#include <cstdint>
#include <iostream>
#include <memory>
#include <stdexcept>

#include "helpers.hpp"

namespace mettle {
  template<typename T>
  std::string to_printable(const compiled_suite<T> &suite) {
    std::ostringstream ss;
    ss << "compiled_suite(" << to_printable(suite.name()) << ", "
       << to_printable(suite.tests()) << ", " << to_printable(suite.subsuites())
       << ")";
    return ss.str();
  }
}

auto match_test(const std::string &name, bool skip) {
  std::ostringstream ss;
  if(skip)
    ss << "skipped ";
  ss << "test named \"" << name << "\"";
  return make_matcher([name, skip](const test_info &actual) {
    if(actual.name != name)
      return false;
    bool skipped = false;
    for(auto &&i : actual.attrs) {
      if(i.attribute.name() == "skip")
        skipped = true;
    }
    return skipped == skip;
  }, ss.str());
}

auto simple_suite(const std::string &suite_name, bool skip_all = false) {
  return all(
    filter([](auto &&x) { return x.name(); }, equal_to(suite_name),
           "name="),
    filter([](auto &&x) { return x.tests(); }, array(
      match_test("test", skip_all), match_test("skipped test", true)
    ), "tests=")
  );
}

auto complex_suite(int skip_level = 4) {
  auto name      = [](auto &&x) { return x.name();      };
  auto tests     = [](auto &&x) { return x.tests();     };
  auto subsuites = [](auto &&x) { return x.subsuites(); };

  auto subsub = all(
    filter(name, equal_to("sub-subsuite"), "name="),
    filter(tests, array(
      match_test("sub-subtest", skip_level < 3),
      match_test("skipped sub-subtest", true)
    ), "tests="),
    filter(subsuites, array(), "subsuites=")
  );

  auto sub = all(
    filter(name, equal_to("subsuite"), "name="),
    filter(tests, array(
      match_test("subtest", skip_level < 2),
      match_test("skipped subtest", true)
    ), "tests="),
    filter(subsuites, array(subsub), "subsuites=")
  );

  return all(
    filter(name, equal_to("test suite"), "name="),
    filter(tests, array(
      match_test("test", skip_level < 1),
      match_test("skipped test", true)
    ), "tests="),
    filter(subsuites, array(sub), "subsuites=")
  );
}

template<typename ...T>
class run_counter {
public:
  run_counter(const std::function<void(T...)> &f = nullptr)
    : f_(f), runs_(std::make_shared<std::size_t>(0)) {}

  void operator ()(T &...t) {
    (*runs_)++;
    if(f_)
      f_(t...);
  }

  std::size_t runs() const {
    return *runs_;
  }
private:
  std::function<void(T...)> f_;
  std::shared_ptr<std::size_t> runs_;
};

struct basic_fixture {
  basic_fixture() = default;
  basic_fixture(const basic_fixture &) = delete;
  basic_fixture & operator =(const basic_fixture &) = delete;

  int data;
};

struct basic_factory {
  template<typename T>
  T make() {
    return { data };
  }

  int data = 0;
};

suite<> test_suite("suite creation", [](auto &_) {

  _.test("create a test suite", []() {
    auto s = make_suite<>("test suite", [](auto &_){
      _.test("test", []() {});
      _.test("skipped test", {skip}, []() {});
    });

    expect(s, simple_suite("test suite"));

    auto s2 = make_suite("test suite", [](auto &_){
      _.test("test", []() {});
      _.test("skipped test", {skip}, []() {});
    });

    expect(s, simple_suite("test suite"));
  });

  _.test("create a test suite with fixture", []() {
    auto s = make_suite<int>("test suite", [](auto &_){
      _.test("test", [](int &) {});
      _.test("skipped test", {skip}, [](int &) {});
    });

    expect(s, simple_suite("test suite"));
  });

  _.test("create a test suite with type-only fixture", []() {
    auto s = make_suite<int>("test suite", type_only, [](auto &_){
      _.test("test", []() {});
      _.test("skipped test", {skip}, []() {});
    });

    expect(s, simple_suite("test suite"));
  });

  _.test("create a test suite with setup/teardown", []() {
    auto s = make_suite<>("test suite", [](auto &_){
      _.setup([]() {});
      _.teardown([]() {});
      _.test("test", []() {});
      _.test("skipped test", {skip}, []() {});
    });

    expect(s, simple_suite("test suite"));
  });

  _.test("create a test suite with fixture and setup/teardown", []() {
    auto s = make_suite<int>("test suite", [](auto &_){
      _.setup([](int &) {});
      _.teardown([](int &) {});
      _.test("test", [](int &) {});
      _.test("skipped test", {skip}, [](int &) {});
    });

    expect(s, simple_suite("test suite"));
  });

  _.test("create a parameterized test suite", []() {
    auto suites = make_suites<int, float>("test suite", [](auto &_) {
      using Fixture = fixture_type_t<decltype(_)>;

      _.test("test", [](auto &) {});
      _.test("skipped test", {skip}, [](auto &) {});
    });

    expect(suites, array(
      simple_suite("test suite (int)"),
      simple_suite("test suite (float)")
    ));
  });

  _.test("create a type-only parameterized test suite", []() {
    auto suites = make_suites<int, float>("test suite", type_only,
                                          [](auto &_) {
      using Fixture = fixture_type_t<decltype(_)>;

      _.test("test", []() {});
      _.test("skipped test", {skip}, []() {});
    });

    expect(suites, array(
      simple_suite("test suite (int)"),
      simple_suite("test suite (float)")
    ));
  });

  _.test("create a test suite via make_suites", []() {
    auto suites = make_suites<>("test suite", [](auto &_) {
      _.test("test", []() {});
      _.test("skipped test", {skip}, []() {});
    });

    expect(suites, array(simple_suite("test suite")));
  });

  _.test("create a test suite with fixture via make_suites", []() {
    auto suites = make_suites<int>("test suite", [](auto &_) {
      _.test("test", [](int &) {});
      _.test("skipped test", {skip}, [](int &) {});
    });

    expect(suites, array(simple_suite("test suite")));
  });

  _.test("create a test suite with type-only fixture via make_suites",
         []() {
    auto suites = make_suites<int>("test suite", type_only, [](auto &_) {
      _.test("test", []() {});
      _.test("skipped test", {skip}, []() {});
    });

    expect(suites, array(simple_suite("test suite")));
  });

  _.test("create a test suite that throws", []() {
    auto make_bad_suite = []() {
      auto s = make_suite<>("broken test suite", [](auto &){
        throw std::runtime_error("bad");
      });
    };

    expect(make_bad_suite, thrown<std::runtime_error>("bad"));
  });

  subsuite<>(_, "subsuite creation", [](auto &_) {

    _.test("create subsuites", []() {
      auto s = make_suite<>("test suite", [](auto &_){
        _.test("test", []() {});
        _.test("skipped test", {skip}, []() {});

        _.template subsuite<int>("subsuite", [](auto &_) {
          _.test("subtest", [](int &) {});
          _.test("skipped subtest", {skip}, [](int &) {});

          _.template subsuite<>("sub-subsuite", [](auto &_) {
            _.test("sub-subtest", [](int &) {});
            _.test("skipped sub-subtest", {skip}, [](int &) {});
          });
        });
      });

      expect(s, complex_suite());
    });

    _.test("create subsuites with helper syntax", []() {
      auto s = make_suite<>("test suite", [](auto &_){
        _.test("test", []() {});
        _.test("skipped test", {skip}, []() {});

        subsuite<int>(_, "subsuite", [](auto &_) {
          _.test("subtest", [](int &) {});
          _.test("skipped subtest", {skip}, [](int &) {});

          subsuite<>(_, "sub-subsuite", [](auto &_) {
            _.test("sub-subtest", [](int &) {});
            _.test("skipped sub-subtest", {skip}, [](int &) {});
          });
        });
      });

      expect(s, complex_suite());
    });

    _.test("create subsuites with make_subsuite", []() {
      auto s = make_suite<>("test suite", [](auto &_){
        _.test("test", []() {});
        _.test("skipped test", {skip}, []() {});

        _.subsuite(make_subsuite<int>(_, "subsuite", [](auto &_) {
          _.test("subtest", [](int &) {});
          _.test("skipped subtest", {skip}, [](int &) {});

          _.subsuite(make_subsuite<>(_, "sub-subsuite", [](auto &_) {
            _.test("sub-subtest", [](int &) {});
            _.test("skipped sub-subtest", {skip}, [](int &) {});
          }));
        }));
      });

      expect(s, complex_suite());
    });

    _.test("create a parameterized subsuite", []() {
      auto s = make_suite<>("test suite", [](auto &_) {
        _.template subsuite<int, float>("subsuite", [](auto &_) {
          using Fixture = fixture_type_t<decltype(_)>;

          _.test("test", [](auto &) {});
          _.test("skipped test", {skip}, [](auto &) {});
        });
      });

      expect(s.name(), equal_to("test suite"));
      expect(s.tests(), array());
      expect(s.subsuites(), array(
        simple_suite("subsuite (int)"),
        simple_suite("subsuite (float)")
      ));
    });

    _.test("create a parameterized subsuite with helper syntax", []() {
      auto s = make_suite<>("test suite", [](auto &_) {
        subsuite<int, float>(_, "subsuite", [](auto &_) {
          using Fixture = fixture_type_t<decltype(_)>;

          _.test("test", [](auto &) {});
          _.test("skipped test", {skip}, [](auto &) {});
        });
      });

      expect(s.name(), equal_to("test suite"));
      expect(s.tests(), array());
      expect(s.subsuites(), array(
        simple_suite("subsuite (int)"),
        simple_suite("subsuite (float)")
      ));
    });

    _.test("create a parameterized subsuite with make_subsuites", []() {
      auto s = make_suite<>("test suite", [](auto &_) {
        _.subsuite(make_subsuites<int, float>(_, "subsuite", [](auto &_) {
          using Fixture = fixture_type_t<decltype(_)>;

          _.test("test", [](auto &) {});
          _.test("skipped test", {skip}, [](auto &) {});
        }));
      });

      expect(s.name(), equal_to("test suite"));
      expect(s.tests(), array());
      expect(s.subsuites(), array(
        simple_suite("subsuite (int)"),
        simple_suite("subsuite (float)")
      ));
    });

  });

  subsuite<>(_, "skipped suites", [](auto &_) {

    _.test("create a skipped test suite", []() {
      auto s = make_suite<>("test suite", {skip}, [](auto &_){
        _.test("test", []() {});
        _.test("skipped test", {skip}, []() {});
      });

      expect(s, simple_suite("test suite", true));
    });

    _.test("create a skipped test suite with fixture", []() {
      auto s = make_suite<int>("test suite", {skip}, [](auto &_){
        _.test("test", [](int &) {});
        _.test("skipped test", {skip}, [](int &) {});
      });

      expect(s, simple_suite("test suite", true));
    });

    _.test("create a skipped parameterized test suite", []() {
      auto suites = make_suites<int, float>("test suite", {skip},
                                            [](auto &_){
        using Fixture = fixture_type_t<decltype(_)>;

        _.test("test", [](auto &) {});
        _.test("skipped test", {skip}, [](auto &) {});
      });

      expect(suites, array(
        simple_suite("test suite (int)", true),
        simple_suite("test suite (float)", true)
      ));
    });

    _.test("create skipped subsuites", []() {
      auto s = make_suite<>("test suite", [](auto &_){
        _.test("test", []() {});
        _.test("skipped test", {skip}, []() {});

        _.template subsuite<int>("subsuite", {skip}, [](auto &_) {
          _.test("subtest", [](int &) {});
          _.test("skipped subtest", {skip}, [](int &) {});

          _.template subsuite<>("sub-subsuite", [](auto &_) {
            _.test("sub-subtest", [](int &) {});
            _.test("skipped sub-subtest", {skip}, [](int &) {});
          });
        });
      });

      expect(s, complex_suite(1));
    });

    _.test("create skipped subsuites with helper syntax", []() {
      auto s = make_suite<>("test suite", [](auto &_){
        _.test("test", []() {});
        _.test("skipped test", {skip}, []() {});

        subsuite<int>(_, "subsuite", {skip}, [](auto &_) {
          _.test("subtest", [](int &) {});
          _.test("skipped subtest", {skip}, [](int &) {});

          subsuite<>(_, "sub-subsuite", [](auto &_) {
            _.test("sub-subtest", [](int &) {});
            _.test("skipped sub-subtest", {skip}, [](int &) {});
          });
        });
      });

      expect(s, complex_suite(1));
    });

    _.test("create skipped subsuites with make_subsuite", []() {
      auto s = make_suite<>("test suite", [](auto &_){
        _.test("test", []() {});
        _.test("skipped test", {skip}, []() {});

        _.subsuite(make_subsuite<int>(_, "subsuite", {skip}, [](auto &_) {
          _.test("subtest", [](int &) {});
          _.test("skipped subtest", {skip}, [](int &) {});

          _.subsuite(make_subsuite<>(_, "sub-subsuite", [](auto &_) {
            _.test("sub-subtest", [](int &) {});
            _.test("skipped sub-subtest", {skip}, [](int &) {});
          }));
        }));
      });

      expect(s, complex_suite(1));
    });

  });

});

suite<> test_calling("test calling", [](auto &_) {

  _.test("passing test called", []() {
    run_counter<> test;
    auto s = make_suite<>("inner", [&test](auto &_){
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect(result.passed, equal_to(true));

    expect(test.runs(), equal_to(1));
  });

  _.test("failing test called", []() {
    run_counter<> test([]() {
      expect(false, equal_to(true));
    });
    auto s = make_suite<>("inner", [&test](auto &_){
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect(result.passed, equal_to(false));

    expect(test.runs(), equal_to(1));
  });

  _.test("setup and teardown called", []() {
    run_counter<> setup, teardown, test;
    auto s = make_suite<>("inner", [&setup, &teardown, &test](auto &_){
      _.setup(setup);
      _.teardown(teardown);
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect(result.passed, equal_to(true));

    expect(setup.runs(), equal_to(1));
    expect(test.runs(), equal_to(1));
    expect(teardown.runs(), equal_to(1));
  });

  _.test("teardown called when test fails", []() {
    run_counter<> setup, teardown;
    run_counter<> test([]() {
      expect(false, equal_to(true));
    });
    auto s = make_suite<>("inner", [&setup, &teardown, &test](auto &_){
      _.setup(setup);
      _.teardown(teardown);
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect(result.passed, equal_to(false));

    expect(setup.runs(), equal_to(1));
    expect(test.runs(), equal_to(1));
    expect(teardown.runs(), equal_to(1));
  });

  _.test("teardown not called when setup fails", []() {
    run_counter<> setup([]() {
      expect(false, equal_to(true));
    });
    run_counter<> teardown, test;
    auto s = make_suite<>("inner", [&setup, &teardown, &test](auto &_){
      _.setup(setup);
      _.teardown(teardown);
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect(result.passed, equal_to(false));

    expect(setup.runs(), equal_to(1));
    expect(test.runs(), equal_to(0));
    expect(teardown.runs(), equal_to(0));
  });

  _.test("test fails when teardown fails", []() {
    run_counter<> teardown([]() {
      expect(false, equal_to(true));
    });
    run_counter<> setup, test;
    auto s = make_suite<>("inner", [&setup, &teardown, &test](auto &_){
      _.setup(setup);
      _.teardown(teardown);
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect(result.passed, equal_to(false));

    expect(setup.runs(), equal_to(1));
    expect(test.runs(), equal_to(1));
    expect(teardown.runs(), equal_to(1));
  });

});

suite<basic_fixture> test_fixtures("suite fixtures", [](auto &_) {

  _.template subsuite<int>("subsuite", type_only, [](auto &_) {
    _.setup([](basic_fixture &f) {
      f.data++;
    });

    _.test("outer fixture was passed by reference", [](basic_fixture &f) {
      expect(f.data, equal_to(2));
    });

    _.template subsuite<basic_fixture>("sub-subsuite", basic_factory{5},
                                       [](auto &_) {
      _.setup([](basic_fixture &f, basic_fixture &) {
        f.data++;
      });

      _.test("outer fixture was passed by reference",
             [](basic_fixture &f, basic_fixture &) {
        expect(f.data, equal_to(3));
      });

      _.test("inner fixture was constructed by factory",
             [](basic_fixture &, basic_fixture &f2) {
        expect(f2.data, equal_to(5));
      });

    });

  });

  // Put the setup after the subsuite is created to ensure that order doesn't
  // matter.
  _.setup([](basic_fixture &f) {
    f.data = 1;
  });

});
