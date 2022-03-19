#include <cassert>

#include "Enums.hpp"

Direction oposite(Direction direction)
{
  switch (direction)
  {
  case UP:
    return DOWN;
  case DOWN:
    return UP;
  case LEFT:
    return RIGHT;
  case RIGHT:
    return LEFT;
  default:
    assert(false);
  }
}

char const *to_string(Direction direction)
{
  static char const *const lookup[] = {
    "UP", "DOWN", "LEFT", "RIGHT"
  };

  return direction < DIRECTION_COUNT ?
                     lookup[direction] : nullptr;
}

char const *to_string(SearchAlgorithm algorithm)
{
  static char const *const lookup[] = {
    "Breath First",
    "Depth First (With cycle detection)",
    "Iterative Deepening",
    "A* (Manhattan Distance)",
    "A* (Number Of Pieces Out Of Place)",
    "Greedy (Manhattan Distance)",
    "Greedy (Number Of Pieces Out Of Place)"
  };

  return algorithm < ALGORITHM_COUNT ? lookup[algorithm] : nullptr;
}

char const *to_string(SearchStatus status)
{
  static char const *const lookup[] = {
    "Got Solution",
    "No Solution",
    "Exceeded Memory Limit",
    "Unsolvable"
  };

  return status < STATUS_COUNT ? lookup[status] : nullptr;
}
