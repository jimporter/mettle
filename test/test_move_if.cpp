#include <mettle.hpp>
using namespace mettle;

struct copyable_type {
  copyable_type() = default;
  copyable_type(const copyable_type &m)
    : copies(m.copies + 1) {}

  int copies = 0;
};

struct moveable_type {
  moveable_type() = default;
  moveable_type(const moveable_type &m)
    : copies(m.copies + 1), moves(m.moves) {}
  moveable_type(moveable_type &&m)
    : copies(m.copies), moves(m.moves + 1) {}

  int copies = 0, moves = 0;
};

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
