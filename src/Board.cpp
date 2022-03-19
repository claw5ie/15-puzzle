#include <algorithm>
#include <cassert>

#include "Board.hpp"

void Board::set_pos()
{
  for (size_t i = 0; i < 4 * 4; i++)
  {
    if (data[i] == EMPTY_SPACE_VALUE)
      pos = i;
    else
      assert(data[i] < 15);
  }
}

bool Board::can_be_solved(Board const &target) const
{
  size_t this_inversions = 0;
  size_t target_inversions = 0;

  for (size_t i = 0; i < 4 * 4; i++)
  {
    for (size_t j = i + 1; j < 4 * 4; j++)
    {
      this_inversions +=
        (data[i] > data[j]) && i != pos && j != pos;
      target_inversions +=
        (target.data[i] > target.data[j]) && i != target.pos && j != target.pos;
    }
  }

  return ((this_inversions % 2) != (pos / 4) % 2) ==
    ((target_inversions % 2) != (target.pos / 4) % 2);
}

bool Board::can_move(Direction direction) const
{
  switch (direction)
  {
  case UP:
    return pos > 3;
  case DOWN:
    return pos < 12;
  case LEFT:
    return pos % 4 != 0;
  case RIGHT:
    return pos % 4 != 3;
  default:
    assert(false);
  }
}

void Board::move(Direction direction)
{
  assert(can_move(direction));

  switch (direction)
  {
  case UP:
    std::swap(data[pos], data[pos - 4]);
    pos -= 4;
    break;
  case DOWN:
    std::swap(data[pos], data[pos + 4]);
    pos += 4;
    break;
  case LEFT:
    std::swap(data[pos], data[pos - 1]);
    pos--;
    break;
  case RIGHT:
    std::swap(data[pos], data[pos + 1]);
    pos++;
    break;
  default:
    assert(false);
  }
}

// Expected the same value for empty space.
bool Board::is_equal(Board const &other) const
{
  for (size_t i = 0; i < 4 * 4; i++)
  {
    if (this->data[i] != other.data[i])
      return false;
  }

  return true;
}

size_t count_pieces_out_of_place(Board const &source, Board const &target)
{
  size_t count = 0;

  for (size_t i = 0; i < 4 * 4; i++)
    count += source.data[i] != target.data[i];

  return count;
}

size_t sum_of_manhattan_distances(Board const &source, Board const &target)
{
  static char lookup[4 * 4];

  for (size_t i = 0; i < 4 * 4; i++)
    lookup[target.data[i]] = i;

  size_t distance = 0;

  for (size_t i = 0; i < 4 * 4; i++)
  {
    int const expected = lookup[source.data[i]];
    distance += std::abs((int)i / 4 - (int)expected / 4) +
      std::abs((int)i % 4 - (int)expected % 4);
  }

  return distance;
}
