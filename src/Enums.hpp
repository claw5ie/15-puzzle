#ifndef ENUMS_HPP
#define ENUMS_HPP

enum Direction
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
  DIRECTION_COUNT
};

Direction oposite(Direction direction);

char const *to_string(Direction direction);

enum SearchAlgorithm
{
  BREATH_FIRST,
  DEPTH_NO_CYCLES_FIRST,
  ITERATIVE_DEEPENING,
  A_STAR_MANHATTAN,
  A_STAR_PIECES_OUT_OF_PLACE,
  GREEDY_MANHATTAN,
  GREEDY_PIECES_OUT_OF_PLACE,
  ALGORITHM_COUNT,
};

char const *to_string(SearchAlgorithm algorithm);

enum SearchStatus
{
  SOLVED,
  NOT_SOLVED,
  MEMORY_EXCEEDED,
  UNSOLVABLE,
  STATUS_COUNT
};

char const *to_string(SearchStatus status);

#endif // ENUMS_HPP
