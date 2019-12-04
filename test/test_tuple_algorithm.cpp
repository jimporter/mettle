#include <mettle.hpp>
using namespace mettle;

#include <sstream>

suite<> test_tuple_alg("tuple algorithms", [](auto &_) {
  subsuite<>(_, "tuple_for_until()", [](auto &_) {
    using detail::tuple_for_each;

    _.test("empty tuple", []() {
      std::tuple<> tup;
      std::size_t count = 0;
      tuple_for_each(tup, [&count](auto &&) {
        count++;
      });

      expect("number of iterations", count, equal_to(0));
    });

    _.test("1-tuple", []() {
      std::tuple<int> tup(1);
      std::size_t count = 0;
      tuple_for_each(tup, [&count](auto &&item) {
        expect("tuple value", item, equal_to(1));
        count++;
      });

      expect("number of iterations", count, equal_to(1));
    });

    _.test("2-tuple", []() {
      std::tuple<int, std::string> tup(1, "two");
      std::size_t count = 0;
      tuple_for_each(tup, [&count](auto &&) {
        count++;
      });

      expect("number of iterations", count, equal_to(2));

      std::tuple<int, int> tup2(0, 1);
      count = 0;
      tuple_for_each(tup2, [&count](auto &&item) {
        expect("tuple value", item, equal_to(count));
        count++;
      });

      expect("number of iterations", count, equal_to(2));
    });

    _.test("pair", []() {
      std::pair<int, std::string> pair(1, "two");
      std::size_t count = 0;
      tuple_for_each(pair, [&count](auto &&) {
        count++;
      });

      expect("number of iterations", count, equal_to(2));
    });

    _.test("early exit", []() {
      std::tuple<int, std::string> tup(1, "two");
      std::size_t count = 0;
      tuple_for_each(tup, [&count](auto &&) {
        count++;
        return true;
      });

      expect("number of iterations", count, equal_to(1));
    });

    _.test("modification", []() {
      std::tuple<int, int> tup(1, 2);
      tuple_for_each(tup, [](auto &&item) {
        item++;
      });

      expect(tup, equal_to(std::make_tuple(2, 3)));
    });
  });

  subsuite<>(_, "tuple_joined()", [](auto &_) {
    using detail::tuple_joined;

    _.test("empty tuple", []() {
      std::tuple<> tup;

      std::ostringstream os1;
      os1 << tuple_joined(tup, [](auto &&) { return "bad"; });
      expect(os1.str(), equal_to(""));

      std::ostringstream os2;
      os2 << tuple_joined(tup, [](auto &&) { return "bad"; }, " and ");
      expect(os2.str(), equal_to(""));
    });

    _.test("1-tuple", []() {
      std::tuple<int> tup(1);

      std::ostringstream os1;
      os1 << tuple_joined(tup, [](auto &&item) { return item; });
      expect(os1.str(), equal_to("1"));

      std::ostringstream os2;
      os2 << tuple_joined(tup, [](auto &&item) { return item; }, " and ");
      expect(os2.str(), equal_to("1"));
    });

    _.test("2-tuple", []() {
      std::tuple<int, std::string> tup(1, "two");

      std::ostringstream os1;
      os1 << tuple_joined(tup, [](auto &&item) { return item; });
      expect(os1.str(), equal_to("1, two"));

      std::ostringstream os2;
      os2 << tuple_joined(tup, [](auto &&item) { return item; }, " and ");
      expect(os2.str(), equal_to("1 and two"));
    });

    _.test("pair", []() {
      std::pair<int, std::string> pair(1, "two");

      std::ostringstream os1;
      os1 << tuple_joined(pair, [](auto &&item) { return item; });
      expect(os1.str(), equal_to("1, two"));

      std::ostringstream os2;
      os2 << tuple_joined(pair, [](auto &&item) { return item; }, " and ");
      expect(os2.str(), equal_to("1 and two"));
    });
  });
});
