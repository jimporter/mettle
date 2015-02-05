#ifndef INC_METTLE_TEST_COPY_COUNTER_HPP
#define INC_METTLE_TEST_COPY_COUNTER_HPP

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

#endif
