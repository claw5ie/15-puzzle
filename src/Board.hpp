#ifndef BOARD_HPP
#define BOARD_HPP

#include <cstdint>
#include <cstddef>

#include "Enums.hpp"

#define EMPTY_SPACE_VALUE 15

struct Board
{
  uint32_t pos;
  uint8_t data[4 * 4];

  bool can_be_solved(Board const &target) const;

  bool can_move(Direction direction) const;

  void move(Direction direction);

  // Expected the same value for empty space.
  bool is_equal(Board const &other) const;
};

size_t count_pieces_out_of_place(Board const &source, Board const &target);

size_t sum_of_manhattan_distances(Board const &source, Board const &target);

#endif // BOARD_HPP
