#include <mettle.hpp>
using namespace mettle;

#include "copy_counter.hpp"

suite<> test_move_if("move_if()", [](auto &_) {
  subsuite<int, int &, const int &>(_, "move_if", type_only, [](auto &_) {
    using Fixture = fixture_type_t<decltype(_)>;

    _.test("move_if<T>(moveable)", []() {
      bool should_move = !std::is_lvalue_reference<Fixture>::value;
      moveable_type from;
      auto to = detail::move_if<Fixture>(from);

      expect("number of copies", to.copies, equal_to(should_move ? 0 : 1));
      expect("number of moves", to.moves, equal_to(should_move ? 1 : 0));
    });

    _.test("move_if<T>(copyable)", []() {
      copyable_type from;
      auto to = detail::move_if<Fixture>(from);

      expect("number of copies", to.copies, equal_to(1));
    });
  });
});
