#include <mettle.hpp>
using namespace mettle;

#include <cstdint>
#include <sstream>
#include <stdexcept>

#include "../helpers.hpp"

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

auto simple_suite(const std::string &suite_name, bool skip_all = false) {
  attributes maybe_skip;
  if(skip_all) maybe_skip.insert(skip);

  return all(
    filter([](auto &&x) { return x.name(); }, equal_to(suite_name),
           "name="),
    filter([](auto &&x) { return x.tests(); }, array(
      equal_test_info("test", maybe_skip),
      equal_test_info("skipped test", {skip})
    ), "tests=")
  );
}

auto complex_suite(int skip_level = 4) {
  attributes do_skip = {skip}, dont_skip = {};

  auto name      = [](auto &&x) { return x.name();      };
  auto tests     = [](auto &&x) { return x.tests();     };
  auto subsuites = [](auto &&x) { return x.subsuites(); };

  auto subsub = all(
    filter(name, equal_to("sub-subsuite"), "name="),
    filter(tests, array(
      equal_test_info("sub-subtest", skip_level < 3 ? do_skip : dont_skip),
      equal_test_info("skipped sub-subtest", do_skip)
    ), "tests="),
    filter(subsuites, array(), "subsuites=")
  );

  auto sub = all(
    filter(name, equal_to("subsuite"), "name="),
    filter(tests, array(
      equal_test_info("subtest", skip_level < 2 ? do_skip : dont_skip),
      equal_test_info("skipped subtest", do_skip)
    ), "tests="),
    filter(subsuites, array(subsub), "subsuites=")
  );

  return all(
    filter(name, equal_to("test suite"), "name="),
    filter(tests, array(
      equal_test_info("test", skip_level < 1 ? do_skip : dont_skip),
      equal_test_info("skipped test", do_skip)
    ), "tests="),
    filter(subsuites, array(sub), "subsuites=")
  );
}

suite<> test_suite("suite creation", [](auto &_) {

  subsuite<>(_, "basic creation", [](auto &_) {
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

    _.test("create a test suite that throws", []() {
      auto make_bad_suite = []() {
        auto s = make_suite<>("broken test suite", [](auto &){
          throw std::runtime_error("bad");
        });
      };

      expect(make_bad_suite, thrown<std::runtime_error>("bad"));
    });

  });

  subsuite<>(_, "subsuite creation", [](auto &_) {

// XXX: Enable this test on MSVC (`.template` triggers an ICE).
#if !defined(_MSC_VER) || defined(__clang__)
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
#endif

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


// XXX: Enable this test on MSVC (`.template` triggers an ICE).
#if !defined(_MSC_VER) || defined(__clang__)
    _.test("create a parameterized subsuite", []() {
      auto s = make_suite<>("test suite", [](auto &_) {
        _.template subsuite<int, float>("subsuite", [](auto &_) {
          using Fixture = fixture_type_t<decltype(_)>;
          (void)Fixture{};

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
#endif

    _.test("create a parameterized subsuite with helper syntax", []() {
      auto s = make_suite<>("test suite", [](auto &_) {
        subsuite<int, float>(_, "subsuite", [](auto &_) {
          using Fixture = fixture_type_t<decltype(_)>;
          (void)Fixture{};

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
          (void)Fixture{};

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

    _.test("create a skipped type-only test suite with fixture", []() {
      auto s = make_suite<int>("test suite", {skip}, type_only, [](auto &_){
        _.test("test", []() {});
        _.test("skipped test", {skip}, []() {});
      });

      expect(s, simple_suite("test suite", true));
    });

// XXX: Enable this test on MSVC (`.template` triggers an ICE).
#if !defined(_MSC_VER) || defined(__clang__)
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
#endif

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

  subsuite<>(_, "parameterized suites", [](auto &_) {
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

    _.test("create a test suite with type-only fixture via make_suites", []() {
      auto suites = make_suites<int>("test suite", type_only, [](auto &_) {
        _.test("test", []() {});
        _.test("skipped test", {skip}, []() {});
      });

      expect(suites, array(simple_suite("test suite")));
    });

    _.test("create a parameterized test suite", []() {
      auto suites = make_suites<int, float>("test suite", [](auto &_) {
        using Fixture = fixture_type_t<decltype(_)>;
        (void)Fixture{};

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
        (void)Fixture{};

        _.test("test", []() {});
        _.test("skipped test", {skip}, []() {});
      });

      expect(suites, array(
        simple_suite("test suite (int)"),
        simple_suite("test suite (float)")
      ));
    });

    _.test("create a skipped parameterized test suite", []() {
      auto suites = make_suites<int, float>("test suite", {skip}, [](auto &_){
        using Fixture = fixture_type_t<decltype(_)>;
        (void)Fixture{};

        _.test("test", [](auto &) {});
        _.test("skipped test", {skip}, [](auto &) {});
      });

      expect(suites, array(
        simple_suite("test suite (int)", true),
        simple_suite("test suite (float)", true)
      ));
    });

    _.test("create a skipped type-only parameterized test suite", []() {
      auto suites = make_suites<int, float>("test suite", {skip}, type_only,
                                            [](auto &_) {
        using Fixture = fixture_type_t<decltype(_)>;
        (void)Fixture{};

        _.test("test", []() {});
        _.test("skipped test", {skip}, []() {});
      });

      expect(suites, array(
        simple_suite("test suite (int)", true),
        simple_suite("test suite (float)", true)
      ));
    });

  });

});

