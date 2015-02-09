#include <mettle.hpp>
using namespace mettle;

#include <sstream>
#include <stdexcept>
#include <vector>

struct some_type {
  some_type(const some_type &) = delete;
  some_type & operator =(const some_type &) = delete;
};

template<typename T>
T about_one() {
  T value = 0;
  for(int i = 0; i < 10; i++)
    value += 0.1L;
  return value;
}

auto msg_matcher(bool match, std::string message = "message") {
  match_result result = {match, std::move(message)};
  return make_matcher([result = std::move(result)](const auto &) {
    return result;
  }, "");
}

auto meta_matcher(int x) {
  return filter([](auto &&i) { return i / 2; }, equal_to(x));
};

suite<> test_matchers("matchers", [](auto &_) {

  subsuite<>(_, "basic", [](auto &_) {
    _.test("anything()", []() {
      expect(true, anything());
      expect(false, anything());
      expect(123, anything());
      expect(some_type{}, anything());

      expect(anything().desc(), equal_to("anything"));
    });

    _.test("is_not()", []() {
      expect(true, is_not(false));
      expect(false, is_not(true));
      expect(true, is_not(is_not(true)));
      expect(123, is_not(equal_to(100)));

      expect(is_not(123).desc(), equal_to("not 123"));

      expect(is_not(msg_matcher(true))(123).message, equal_to("message"));
      expect(is_not(msg_matcher(false))(123).message, equal_to("message"));
      expect(is_not(msg_matcher(true, ""))(123).message, equal_to(""));
      expect(is_not(msg_matcher(false, ""))(123).message, equal_to(""));
    });

    _.test("describe()", []() {
      expect(true, describe(equal_to(true), "foo"));
      expect(true, is_not(describe(equal_to(false), "foo")));

      expect(describe(equal_to(123), "foo").desc(), equal_to("foo"));
    });

    _.test("filter()", []() {
      std::pair<std::string, int> p("first", 1);
      auto first = [](auto &&x) { return x.first; };
      auto second = [](auto &&x) { return x.second; };
      auto identity = [](auto &&x) { return x; };

      expect(p, filter( first, equal_to("first") ));
      expect(p, filter( second, is_not(less(0)) ));
      expect(p, is_not(filter( second, less(0) )));

      expect(p, filter( first, equal_to("first"), ".first " ));
      expect(p, filter( second, is_not(less(0)), ".second " ));
      expect(p, is_not(filter( second, less(0), ".second " )));

      expect(filter(identity, equal_to(123)).desc(), equal_to("123"));
      expect(filter(identity, equal_to(123), "desc ").desc(),
             equal_to("desc 123"));

      expect(filter(second, equal_to(1))(p).message, equal_to("1"));
      expect(filter(second, msg_matcher(true))(p).message, equal_to("message"));
      expect(filter(second, msg_matcher(false))(p).message,
             equal_to("message"));
      expect(filter(second, msg_matcher(true, ""))(p).message, equal_to("1"));
      expect(filter(second, msg_matcher(false, ""))(p).message, equal_to("1"));

      expect(filter(second, equal_to(1), "desc ")(p).message,
             equal_to("desc 1"));
      expect(filter(second, msg_matcher(true), "desc ")(p).message,
             equal_to("desc message"));
      expect(filter(second, msg_matcher(false), "desc ")(p).message,
             equal_to("desc message"));
      expect(filter(second, msg_matcher(true, ""), "desc ")(p).message,
             equal_to("desc 1"));
      expect(filter(second, msg_matcher(false, ""), "desc ")(p).message,
             equal_to("desc 1"));
    });
  });

  subsuite<>(_, "relational", [](auto &_) {
    _.test("equal_to()", []() {
      expect(true, equal_to(true));
      expect(123, equal_to(123));
      expect("foo", equal_to(std::string("foo")));
      expect(std::string("foo"), equal_to("foo"));
      expect(std::string("foo"), equal_to(std::string("foo")));

      expect(equal_to(123).desc(), equal_to("123"));
    });

    _.test("not_equal_to()", []() {
      expect(true, not_equal_to(false));
      expect(123, not_equal_to(1234));

      expect(not_equal_to(123).desc(), equal_to("not 123"));
    });

    _.test("greater()", []() {
      expect(123, greater(0));

      expect(greater(123).desc(), equal_to("> 123"));
    });

    _.test("greater_equal()", []() {
      expect(123, greater_equal(0));

      expect(greater_equal(123).desc(), equal_to(">= 123"));
    });

    _.test("less()", []() {
      expect(123, less(1000));

      expect(less(123).desc(), equal_to("< 123"));
    });

    _.test("less_equal()", []() {
      expect(123, less_equal(1000));

      expect(less_equal(123).desc(), equal_to("<= 123"));
    });
  });

  subsuite<>(_, "arithmetic", [](auto &_) {
    _.test("near_to()", []() {
      expect(about_one<float>(), near_to(1.0f));
      expect(about_one<double>(), near_to(1.0));
      expect(about_one<long double>(), near_to(1.0L));

      expect(about_one<float>(), near_to(1.0f, 1e-6f));
      expect(about_one<double>(), near_to(1.0, 1e-6));
      expect(about_one<long double>(), near_to(1.0L, 1e-6L));

      expect(std::numeric_limits<float>::quiet_NaN(), is_not(near_to(0.0f)));
      expect(std::numeric_limits<double>::quiet_NaN(), is_not(near_to(0.0)));
      expect(std::numeric_limits<long double>::quiet_NaN(),
             is_not(near_to(0.0L)));

      expect(near_to(1.23f).desc(), equal_to("~= 1.23"));
    });

    _.test("near_to_abs()", []() {
      expect(1.01f, near_to(1.0f, 0.01f));
      expect(1.01, near_to(1.0, 0.01));
      expect(1.01L, near_to(1.0L, 0.01L));

      expect(std::numeric_limits<float>::quiet_NaN(),
             is_not(near_to_abs(0.0f, 0.01f)));
      expect(std::numeric_limits<double>::quiet_NaN(),
             is_not(near_to(0.0, 0.01)));
      expect(std::numeric_limits<long double>::quiet_NaN(),
             is_not(near_to(0.0L, 0.01L)));

      expect(near_to_abs(1.23f, 0.0f).desc(), equal_to("~= 1.23"));
    });
  });

  subsuite<>(_, "combinatoric", [](auto &_) {
    _.test("any()", []() {
      expect(123, any(equal_to(1), equal_to(2), equal_to(123)));
      expect(123, any(1, 2, 123));
      expect(123, is_not(any(1, 2, 3)));
      expect(123, is_not(any()));

      auto m = equal_to(123);
      expect(123, any(m));

      expect(any(1, 2, 3).desc(), equal_to("any of(1, 2, 3)"));

      expect(any(1, msg_matcher(true))(2).message, equal_to("message"));
      expect(any(1, msg_matcher(true, ""))(2).message, equal_to(""));
      expect(any(1, is_not(msg_matcher(true)))(2).message, equal_to(""));
    });

    _.test("all()", []() {
      expect(123, all(123));
      expect(123, all(not_equal_to(1), not_equal_to(2), greater(3)));
      expect(123, all());

      auto m = equal_to(123);
      expect(123, all(m));

      expect(all(1, 2, 3).desc(), equal_to("all of(1, 2, 3)"));

      expect(all(1, msg_matcher(false))(1).message, equal_to("message"));
      expect(all(1, msg_matcher(false, ""))(1).message, equal_to(""));
      expect(all(1, is_not(msg_matcher(false)))(1).message, equal_to(""));
    });

    _.test("none()", []() {
      expect(123, none(1));
      expect(123, none(equal_to(1), equal_to(2), less(3)));
      expect(123, none());

      auto m = equal_to(1);
      expect(123, none(m));

      expect(none(1, 2, 3).desc(), equal_to("none of(1, 2, 3)"));

      expect(none(1, msg_matcher(true))(2).message, equal_to("message"));
      expect(none(1, msg_matcher(true, ""))(2).message, equal_to(""));
      expect(none(1, is_not(msg_matcher(true)))(2).message, equal_to(""));
    });
  });

  subsuite<>(_, "collection", [](auto &_) {
    _.test("member()", []() {
      expect(std::vector<int>{}, is_not(member(0)));
      expect(std::vector<int>{1, 2, 3}, member(1));
      expect(std::vector<int>{1, 2, 3}, member(3));
      expect(std::vector<int>{1, 2, 3}, is_not(member(4)));

      int arr[] = {1, 2, 3};
      expect(arr, member(1));
      expect(arr, member(3));
      expect(arr, is_not(member(4)));

      expect(std::vector<int>{}, is_not(member( msg_matcher(true) )));
      expect(std::vector<int>{1, 2, 3}, member(
        filter([](auto &&i) { return 2 * i; }, equal_to(2))
      ));
      expect(std::vector<int>{1, 2, 3}, member(
        filter([](auto &&i) { return 2 * i; }, equal_to(6))
      ));
      expect(std::vector<int>{1, 2, 3}, is_not(member(
        filter([](auto &&i) { return 2 * i; }, equal_to(1))
      )));

      expect(member(123).desc(), equal_to("member 123"));
      expect(member(123)(arr).message, equal_to("[1, 2, 3]"));
      expect(member(msg_matcher(true))(arr).message,
             equal_to("[message, message, message]"));
      expect(each(msg_matcher(true, ""))(arr).message,
             equal_to("[1, 2, 3]"));
    });

    _.test("each()", []() {
      expect(std::vector<int>{}, each( is_not(anything()) ));
      expect(std::vector<int>{1, 2, 3}, each( greater(0) ));
      expect(std::vector<int>{1, 2, 3}, is_not( each(less(2)) ));

      int arr[] = {1, 2, 3};
      expect(arr, each( greater(0)) );
      expect(arr, is_not( each(less(2)) ));

      expect(std::vector<int>{}, each( msg_matcher(false) ));
      expect(std::vector<int>{1, 2, 3}, each(
        filter([](auto &&i) { return 2 * i; }, greater(0))
      ));
      expect(std::vector<int>{1, 2, 3}, is_not(each(
        filter([](auto &&i) { return 2 * i; }, less(4))
      )));

      expect(each(123).desc(), equal_to("each 123"));
      expect(each(123)(arr).message, equal_to("[1, 2, 3]"));
      expect(each(msg_matcher(true))(arr).message,
             equal_to("[message, message, message]"));
      expect(each(msg_matcher(true, ""))(arr).message,
             equal_to("[1, 2, 3]"));
    });

    _.test("each(begin, end, m)", []() {
      using ivec = std::vector<int>;
      ivec v = {1, 2, 3};

      expect(ivec{}, each(v.begin(), v.begin(), equal_to<const int &>));
      expect(ivec{1, 2, 3}, each(v.begin(), v.end(), equal_to<const int &>));
      expect(ivec{3, 2, 1}, is_not(
        each(v.begin(), v.end(), equal_to<const int &>))
      );
      expect(ivec{1, 2}, is_not(
        each(v.begin(), v.end(), equal_to<const int &>))
      );
      expect(ivec{1, 2, 3, 4}, is_not(
        each(v.begin(), v.end(), equal_to<const int &>))
      );

      expect(v, each(v.begin(), v.end(), equal_to<const int &>));

      int arr[] = {1, 2, 3};
      expect(arr, each(v.begin(), v.end(), equal_to<const int &>));

      expect(ivec{}, each(v.begin(), v.begin(), meta_matcher));
      expect(ivec{2, 4, 6}, each(v.begin(), v.end(), meta_matcher));
      expect(ivec{6, 4, 2}, is_not(each(v.begin(), v.end(), meta_matcher)));
      expect(ivec{2, 4}, is_not(each(v.begin(), v.end(), meta_matcher)));
      expect(ivec{2, 4, 6, 8}, is_not(each(v.begin(), v.end(), meta_matcher)));

      expect(each(
        v.begin(), v.end(), equal_to<const int &>
      )(ivec{2, 4, 6}).message, equal_to("[2, 4, 6]"));
      expect(each(
        v.begin(), v.end(), equal_to<const int &>
      )(ivec{2, 4, 6, 8}).message, equal_to("[2, 4, 6, 8]"));
      expect(each(v.begin(), v.end(), greater<const int &>).desc(),
             equal_to("[> 1, > 2, > 3]"));
      expect(each(v.begin(), v.end(), meta_matcher)(ivec{2, 4, 6}).message,
             equal_to("[1, 2, 3]"));
      expect(each(v.begin(), v.end(), meta_matcher)(ivec{2, 4, 6, 8}).message,
             equal_to("[1, 2, 3, 8]"));
    });

    _.test("each(container, m)", []() {
      using ivec = std::vector<int>;
      ivec v = {1, 2, 3};

      expect(ivec{}, each(ivec{}, equal_to<const int &>));
      expect(ivec{1, 2, 3}, each(v, equal_to<const int &>));
      expect(ivec{3, 2, 1}, is_not(each(v, equal_to<const int &>)));
      expect(ivec{1, 2}, is_not(each(v, equal_to<const int &>)));
      expect(ivec{1, 2, 3, 4}, is_not(each(v, equal_to<const int &>)));

      int arr[] = {1, 2, 3};
      expect(arr, each(v, equal_to<const int &>));

      expect(v, each(ivec{1, 2, 3}, equal_to<int>));
      expect(v, each({1, 2, 3}, equal_to<const int &>));
      expect(v, each(v, equal_to<const int &>));
      expect(v, each(arr, equal_to<const int &>));

      expect(ivec{}, each(ivec{}, meta_matcher));
      expect(ivec{2, 4, 6}, each(v, meta_matcher));
      expect(ivec{6, 4, 2}, is_not(each(v, meta_matcher)));
      expect(ivec{2, 4}, is_not(each(v, meta_matcher)));
      expect(ivec{2, 4, 6, 8}, is_not(each(v, meta_matcher)));

      expect(each({1, 2, 3}, greater<const int &>).desc(),
             equal_to("[> 1, > 2, > 3]"));

      expect(each({1, 2, 3}, equal_to<const int &>)(ivec{2, 4, 6}).message,
             equal_to("[2, 4, 6]"));
      expect(each({1, 2, 3}, equal_to<const int &>)(ivec{2, 4, 6, 8}).message,
             equal_to("[2, 4, 6, 8]"));
      expect(each({1, 2, 3}, meta_matcher)(ivec{2, 4, 6}).message,
             equal_to("[1, 2, 3]"));
      expect(each({1, 2, 3}, meta_matcher)(ivec{2, 4, 6, 8}).message,
             equal_to("[1, 2, 3, 8]"));
    });

    _.test("array()", []() {
      expect(std::vector<int>{}, array());
      expect(std::vector<int>{1, 2, 3}, array(1, 2, 3));
      expect(std::vector<int>{1, 2, 3}, is_not(array(3, 2, 1)));
      expect(std::vector<int>{1, 2, 3}, is_not(array(1, 2)));
      expect(std::vector<int>{1, 2, 3}, is_not(array(1, 2, 3, 4)));

      std::vector<int> vec = {1, 2, 3};
      expect(vec, array(1, 2, 3));
      expect(vec, is_not(array(3, 2, 1)));
      expect(vec, is_not(array(1, 2)));
      expect(vec, is_not(array(1, 2, 3, 4)));

      int arr[] = {1, 2, 3};
      expect(arr, array(1, 2, 3));
      expect(arr, is_not(array(3, 2, 1)));
      expect(arr, is_not(array(1, 2)));
      expect(arr, is_not(array(1, 2, 3, 4)));

      expect(array(1, 2, 3).desc(), equal_to("[1, 2, 3]"));
      expect(array(1, 2, 3)(arr).message, equal_to("[1, 2, 3]"));
      expect(array(msg_matcher(true), msg_matcher(false))(arr).message,
             equal_to("[message, message, 3]"));
    });

    _.test("sorted()", []() {
      expect(std::vector<int>{}, sorted());
      expect(std::vector<int>{1, 2, 3}, sorted());
      expect(std::vector<int>{1, 2, 3}, sorted(std::less<int>()));
      expect(std::vector<int>{3, 2, 1}, sorted(std::greater<int>()));
      expect(std::vector<int>{2, 1, 3}, is_not( sorted()) );
      expect(std::vector<int>{2, 1, 3}, is_not( sorted(std::less<int>()) ));
      expect(std::vector<int>{2, 1, 3}, is_not( sorted(std::greater<int>()) ));

      int arr[] = {1, 2, 3};
      expect(arr, sorted());
      expect(arr, sorted(std::less<int>()));
      expect(arr, is_not( sorted(std::greater<int>()) ));

      expect(sorted().desc(), equal_to("sorted"));
      expect(sorted(std::less<int>()).desc(),
             equal_to("sorted by " + type_name<std::less<int>>()));
    });
  });

  subsuite<>(_, "exception", [](auto &_) {
    auto thrower = []() { throw std::runtime_error("message"); };
    auto int_thrower = []() { throw 123; };
    auto noop = []() {};

    _.test("thrown()", [thrower, int_thrower, noop]() {
      expect(thrower, thrown());
      expect(int_thrower, thrown());
      expect(noop, is_not(thrown()));

      expect(thrown().desc(), equal_to("threw exception"));

      expect(thrown()(noop).message, equal_to("threw nothing"));

      std::string ex_name = type_name<std::runtime_error>();
      expect(is_not(thrown())(thrower).message,
             equal_to("threw " + ex_name + "(\"message\")"));
      expect(is_not(thrown())(int_thrower).message,
             equal_to("threw unknown exception"));
    });

    _.test("thrown<T>()", [thrower, int_thrower, noop]() {
      expect(thrower, thrown<std::runtime_error>());
      expect(thrower, thrown<std::exception>());
      expect(thrower, is_not(thrown<std::logic_error>()));

      expect(int_thrower, thrown<int>());
      expect(int_thrower, is_not(thrown<std::exception>()));

      expect(noop, is_not(thrown<std::exception>()));

      expect(thrown<std::exception>().desc(),
             equal_to("threw " + type_name<std::exception>() + "(anything)"));

      std::string ex_name = type_name<std::runtime_error>();
      expect(thrown<std::logic_error>()(thrower).message,
             equal_to("threw " + ex_name + "(\"message\")"));
      expect(thrown<std::exception>()(int_thrower).message,
             equal_to("threw unknown exception"));
      expect(thrown<std::exception>()(noop).message, equal_to("threw nothing"));

      expect(is_not(thrown<std::runtime_error>())(thrower).message,
             equal_to("threw " + ex_name + "(\"message\")"));
      expect(is_not(thrown<int>())(int_thrower).message,
             equal_to("threw 123"));
    });

    _.test("thrown<T>(what)", [thrower, int_thrower, noop]() {
      expect(thrower, thrown<std::runtime_error>("message"));
      expect(thrower, thrown<std::runtime_error>(is_not("wrong")));
      expect(thrower, is_not(thrown<std::logic_error>( anything() )));

      expect(int_thrower, is_not(thrown<std::exception>( anything() )));

      expect(noop, is_not(thrown<std::exception>( anything() )));

      expect(thrown<std::exception>("message").desc(),
             equal_to("threw " + type_name<std::exception>() +
                      "(what: \"message\")"));

      std::string ex_name = type_name<std::runtime_error>();
      expect(thrown<std::logic_error>("message")(thrower).message,
             equal_to("threw " + ex_name + "(\"message\")"));

      expect(thrown<std::exception>("wrong")(thrower).message,
             equal_to("threw " + ex_name + "(what: \"message\")"));
      expect(thrown<std::exception>("message")(int_thrower).message,
             equal_to("threw unknown exception"));
      expect(thrown<std::exception>("message")(noop).message,
             equal_to("threw nothing"));

      expect(is_not(thrown<std::runtime_error>("message"))(thrower).message,
             equal_to("threw " + ex_name + "(what: \"message\")"));
      expect(is_not(thrown<std::runtime_error>(anything()))(thrower).message,
             equal_to("threw " + ex_name + "(what: \"message\")"));

      expect(thrown<std::runtime_error>(msg_matcher(true))(thrower).message,
             equal_to("threw " + ex_name + "(what: message)"));
    });

    _.test("thrown_raw<T>()", [thrower, int_thrower, noop]() {
      expect(thrower, thrown_raw<std::runtime_error>(anything()));
      expect(thrower, is_not(thrown_raw<std::logic_error>( anything() )));

      expect(int_thrower, thrown_raw<int>(123));
      expect(int_thrower, is_not(thrown_raw<int>(0)));

      expect(noop, is_not(thrown_raw<int>( anything() )));

      expect(thrown_raw<int>(123).desc(),
             equal_to("threw " + type_name<int>() + "(123)"));

      expect(thrown_raw<int>(666)(int_thrower).message,
             equal_to("threw 123"));
      expect(thrown_raw<int>(123)(noop).message,
             equal_to("threw nothing"));

      expect(is_not(thrown_raw<int>(123))(int_thrower).message,
             equal_to("threw 123"));
    });
  });

  subsuite<>(_, "death", [](auto &_) {
    _.test("killed()", []() {
      auto aborter = []() { abort(); };
      auto noop = []() {};

      expect(aborter, killed());
      expect(aborter, killed(SIGABRT));

      expect(noop, is_not(killed()));
      expect(noop, is_not(killed(SIGABRT)));

      expect(killed().desc(), equal_to("killed"));
      expect(killed(SIGABRT).desc(),
             equal_to("killed with signal " + std::to_string(SIGABRT)));

      expect(killed()(noop).message, equal_to("wasn't killed"));
      expect(killed()(aborter).message,
             equal_to("killed with signal " + std::to_string(SIGABRT)));
    });

    _.test("exited()", []() {
      auto exiter = []() { _exit(0); };
      auto noop = []() {};

      expect(exiter, exited());
      expect(exiter, exited(0));

      expect(noop, is_not(exited()));
      expect(noop, is_not(exited(0)));

      expect(exited().desc(), equal_to("exited"));
      expect(exited(0).desc(), equal_to("exited with status 0"));

      expect(exited()(noop).message, equal_to("didn't exit"));
      expect(exited()(exiter).message, equal_to("exited with status 0"));
    });
  });

});
