#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <list>
#include <set>
#include <memory>
#include <functional>
#include <chrono>
#include <cassert>

#define MEMORY_LIMIT ((size_t)1024 * 1024)

enum Direction
{
  UP,
  DOWN,
  LEFT,
  RIGHT,
  COUNT
};

char const *to_string(Direction direction)
{
  static char const *const lookup[] = {
    "UP", "DOWN", "LEFT", "RIGHT"
  };

  return direction < Direction::COUNT ?
                     lookup[direction] : nullptr;
}

Direction oposite(Direction direction)
{
  switch (direction)
  {
  case Direction::UP:
    return Direction::DOWN;
  case Direction::DOWN:
    return Direction::UP;
  case Direction::LEFT:
    return Direction::RIGHT;
  case Direction::RIGHT:
    return Direction::LEFT;
  default:
    assert(false);
  }
}

using Path = std::vector<Direction>;

auto const chrono_default_value = std::chrono::duration<double>();

struct SearchResult
{
  bool found_path;
  Path path;
  size_t explored_nodes_count;
  size_t max_nodes_in_stack;
  std::chrono::duration<double> time_spent;
};

#define EMPTY_SPACE 15

struct Board
{
  uint32_t pos;
  uint8_t data[4 * 4];

  void set_pos()
  {
    for (size_t i = 0; i < 4 * 4; i++)
    {
      if (data[i] == EMPTY_SPACE)
      {
        pos = i;
      }
      else
      {
        assert(data[i] < 15);
      }
    }
  }

  bool can_be_solved(Board const &target) const
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

  bool can_move(Direction direction) const
  {
    switch (direction)
    {
    case Direction::UP:
      return pos > 3;
    case Direction::DOWN:
      return pos < 12;
    case Direction::LEFT:
      return pos % 4 != 0;
    case Direction::RIGHT:
      return pos % 4 != 3;
    default:
      assert(false);
    }
  }

  void move(Direction direction)
  {
    assert(can_move(direction));

    switch (direction)
    {
    case Direction::UP:
      std::swap(data[pos], data[pos - 4]);
      pos -= 4;
      break;
    case Direction::DOWN:
      std::swap(data[pos], data[pos + 4]);
      pos += 4;
      break;
    case Direction::LEFT:
      std::swap(data[pos], data[pos - 1]);
      pos--;
      break;
    case Direction::RIGHT:
      std::swap(data[pos], data[pos + 1]);
      pos++;
      break;
    default:
      assert(false);
    }
  }

  // Expected the same value for empty space.
  bool is_equal(Board const &other) const
  {
    for (size_t i = 0; i < 4 * 4; i++)
    {
      if (this->data[i] != other.data[i])
        return false;
    }

    return true;
  }
};

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

// Change vector to something else to avoid reallocations of large vectors.
SearchResult depth_search_no_cycles(Board const &source, Board const &target)
{
  struct Node
  {
    Board board;
    Path path;
  };

  std::list<Board> explored;
  std::list<Node> to_explore;

  auto const it_repeats_or_was_explored =
    [&to_explore, &explored](Board const &board) -> bool
    {
      for (auto &elem: to_explore)
      {
        if (board.is_equal(elem.board))
          return true;
      }

      for (auto &elem: explored)
      {
        if (board.is_equal(elem))
          return true;
      }

      return false;
    };

  to_explore.push_back({ source, { } });
  size_t max_nodes_in_stack = 1;

  while (!to_explore.empty())
  {
    max_nodes_in_stack = std::max(max_nodes_in_stack, to_explore.size());

    Node node = std::move(to_explore.back());

    if (node.board.is_equal(target))
      return { true, std::move(node.path), explored.size(), max_nodes_in_stack, chrono_default_value };

    if (to_explore.size() * sizeof(Node) > MEMORY_LIMIT)
      return { false, { }, explored.size(), max_nodes_in_stack, chrono_default_value };

    explored.push_back(node.board);
    to_explore.pop_back();

    for (int i = 0; i < Direction::COUNT; i++)
    {
      Direction dir = (Direction)i;

      if (node.board.can_move(dir))
      {
        node.board.move(dir);

        if (!it_repeats_or_was_explored(node.board))
        {
          to_explore.push_back({ node.board, node.path });
          to_explore.back().path.push_back(dir);
        }

        node.board.move(oposite(dir));
      }
    }
  }

  return { false, { }, explored.size(), max_nodes_in_stack, chrono_default_value };
}

SearchResult iterative_deepening_search(
  Board const &source, Board const &target, size_t max_depth
  )
{
  struct Node
  {
    size_t depth;
    Board board;
    Path path;
  };

  std::stack<Node> to_explore;

  // Avoid constant reallocations of stack.
  auto const iterative_deepening_search_aux =
    [&to_explore, &source, &target](size_t depth) -> SearchResult
    {
      size_t explored_nodes_count = 0,
        max_nodes_in_stack = 1;

      to_explore.push({ 0, source, { } });

      while (!to_explore.empty())
      {
        max_nodes_in_stack = std::max(max_nodes_in_stack, to_explore.size());

        Node node = std::move(to_explore.top());

        to_explore.pop();
        explored_nodes_count++;

        if (node.board.is_equal(target))
          return { true, std::move(node.path), explored_nodes_count, max_nodes_in_stack, chrono_default_value };
        else if (node.depth > depth)
          continue;

        for (int i = 0; i < Direction::COUNT; i++)
        {
          Direction dir = (Direction)i;

          if (node.board.can_move(dir))
          {
            node.board.move(dir);

            to_explore.push({ node.depth + 1, node.board, node.path });
            to_explore.top().path.push_back(dir);

            node.board.move(oposite(dir));
          }
        }
      }

      return { false, { }, explored_nodes_count, max_nodes_in_stack, chrono_default_value };
    };

  size_t max_nodes_in_stack = 0,
    explored_nodes_count = 0;

  // Assuming that the stack always grows when the depth increases.
  for (size_t depth = 0; depth <= max_depth; depth++)
  {
    auto res = iterative_deepening_search_aux(depth);

    max_nodes_in_stack = std::max(max_nodes_in_stack, res.max_nodes_in_stack);
    explored_nodes_count = std::max(explored_nodes_count, res.explored_nodes_count);

    if (res.found_path)
      return res;
  }

  return { false, { }, max_nodes_in_stack, explored_nodes_count, chrono_default_value };
}

SearchResult breath_search(Board const &source, Board const &target)
{
  struct Node
  {
    Board board;
    Path path;
  };

  std::queue<Node> to_explore;

  to_explore.push({ source, { } });

  size_t explored_nodes_count = 0,
    max_nodes_in_stack = 1;

  while (!to_explore.empty())
  {
    max_nodes_in_stack = std::max(max_nodes_in_stack, to_explore.size());

    Node node = std::move(to_explore.front());

    to_explore.pop();
    explored_nodes_count++;

    if (node.board.is_equal(target))
      return { true, std::move(node.path), max_nodes_in_stack, explored_nodes_count, chrono_default_value };

    for (int i = 0; i < Direction::COUNT; i++)
    {
      Direction dir = (Direction)i;

      if (node.board.can_move(dir))
      {
        node.board.move(dir);

        to_explore.push({ node.board, node.path });
        to_explore.back().path.push_back(dir);

        node.board.move(oposite(dir));
      }
    }
  }

  return { false, { }, max_nodes_in_stack, explored_nodes_count, chrono_default_value };
}

using HeuristicFunc = size_t (*)(Board const &, Board const &);

SearchResult a_star_search(Board const &source, Board const &target, HeuristicFunc heuristic)
{
  struct Node
  {
    Board board;
    Path path;
  };

  using ComparatorFunc = bool(Node const &, Node const &);

  std::multiset<Node, std::function<ComparatorFunc>> to_explore(
    std::function<ComparatorFunc>(
      [heuristic, &target](Node const &left, Node const &right) -> bool
      {
        return left.path.size() + heuristic(left.board, target) <
          right.path.size() + heuristic(right.board, target);
      }
      ));

  to_explore.insert({ source, { } });
  size_t explored_nodes_count = 0,
    max_nodes_in_stack = 1;

  while (!to_explore.empty())
  {
    max_nodes_in_stack = std::max(max_nodes_in_stack, to_explore.size());

    Node node = std::move(*to_explore.begin());

    to_explore.erase(to_explore.begin());
    explored_nodes_count++;

    if (node.board.is_equal(target))
      return { true, std::move(node.path), max_nodes_in_stack, explored_nodes_count, chrono_default_value };

    for (int i = 0; i < Direction::COUNT; i++)
    {
      Direction dir = (Direction)i;

      if (node.board.can_move(dir))
      {
        node.board.move(dir);

        Node new_node = { node.board, node.path };
        new_node.path.push_back(dir);
        to_explore.insert(std::move(new_node));

        node.board.move(oposite(dir));
      }
    }
  }

  return { false, { }, max_nodes_in_stack, explored_nodes_count, chrono_default_value };
}

SearchResult greedy_search(
  Board const &source, Board const &target, HeuristicFunc heuristic
  )
{
  struct Node
  {
    Board board;
    Path path;
  };

  using ComparatorFunc = bool(Node const &, Node const &);

  std::multiset<Node, std::function<ComparatorFunc>> to_explore(
    std::function<ComparatorFunc>(
      [heuristic, &target](Node const &left, Node const &right) -> bool
      {
        return heuristic(left.board, target) < heuristic(right.board, target);
      }
      ));

  to_explore.insert({ source, { } });

  size_t explored_nodes_count = 0,
    max_nodes_in_stack = 1;

  while (!to_explore.empty())
  {
    max_nodes_in_stack = std::max(max_nodes_in_stack, to_explore.size());

    Node node = std::move(*to_explore.begin());

    to_explore.erase(to_explore.begin());
    explored_nodes_count++;

    if (node.board.is_equal(target))
      return { true, std::move(node.path), max_nodes_in_stack, explored_nodes_count, chrono_default_value };

    if (to_explore.size() > MEMORY_LIMIT)
      return { false, { }, max_nodes_in_stack, explored_nodes_count, chrono_default_value };

    for (int i = 0; i < Direction::COUNT; i++)
    {
      Direction dir = (Direction)i;

      if (node.board.can_move(dir))
      {
        node.board.move(dir);

        Node new_node = { node.board, node.path };
        new_node.path.push_back(dir);
        to_explore.insert(std::move(new_node));

        node.board.move(oposite(dir));
      }
    }
  }

  return { false, { }, max_nodes_in_stack, explored_nodes_count, chrono_default_value };
}

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

#define MAX_DEPTH 32

SearchResult choose_algorithm(
  SearchAlgorithm algorithm, Board const &source, Board const &target
  )
{
  if (!source.can_be_solved(target))
    return { false, { }, 0, 0, chrono_default_value };

  SearchResult res;
  auto const start = std::chrono::steady_clock::now();

  switch (algorithm)
  {
  case BREATH_FIRST:
    res = breath_search(source, target);
    break;
  case DEPTH_NO_CYCLES_FIRST:
    res = depth_search_no_cycles(source, target);
    break;
  case ITERATIVE_DEEPENING:
    res = iterative_deepening_search(source, target, MAX_DEPTH);
    break;
  case A_STAR_MANHATTAN:
    res = a_star_search(source, target, sum_of_manhattan_distances);
    break;
  case A_STAR_PIECES_OUT_OF_PLACE:
    res = a_star_search(source, target, count_pieces_out_of_place);
    break;
  case GREEDY_MANHATTAN:
    res = greedy_search(source, target, sum_of_manhattan_distances);
    break;
  case GREEDY_PIECES_OUT_OF_PLACE:
    res = greedy_search(source, target, count_pieces_out_of_place);
    break;
  default:
    assert(false);
  }

  res.time_spent = std::chrono::steady_clock::now() - start;

  return res;
}

int main()
{
  Board sources[] = {
    { 0, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 15, 10, 11, 12, 9, 13, 14 }},
    { 0, { 0, 1, 2, 3, 4, 5, 7, 11, 12, 8, 15, 6, 13, 10, 9, 14 }},
    { 0, { 0, 1, 2, 3, 12, 5, 7, 11, 4, 8, 15, 6, 13, 10, 9, 14 }}
  };

  Board target = { 15, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }};

  for (size_t i = 0; i < sizeof(sources) / sizeof(Board); i++)
    sources[i].set_pos();

  target.set_pos();

  for (size_t i = 0; i < sizeof(sources) / sizeof(Board); i++)
  {
    std::cout << "\n\n";

    for (int j = 0; j < ALGORITHM_COUNT; j++)
    {
      std::cout << "Chosen algorithm: " << to_string((SearchAlgorithm)j) << '\n';

      auto res = choose_algorithm((SearchAlgorithm)j, sources[i], target);

      if (res.found_path)
      {
        std::cout << " * Found solution:  ";

        size_t i = 0;
        for (auto &dir: res.path)
        {
          std::cout << to_string(dir);

          if (++i < res.path.size())
            std::cout << ' ';
        }

      }
      else
      {
        std::cout << " * Didn't find anything :(";
      }

      std::cout << "\n   - Spent          " << res.time_spent.count() << " seconds\n"
                << "   - Took           " << res.path.size() << " moves\n"
                << "   - Explored       " << res.explored_nodes_count << " nodes in total\n"
                << "   - Stored at most " << res.max_nodes_in_stack <<  " nodes in stack\n";
    }
  }
}
