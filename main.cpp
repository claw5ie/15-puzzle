#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <set>
#include <memory>
#include <functional>
#include <cassert>

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

struct SearchResult
{
  bool found_path;
  Path path;
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

  std::vector<Board> explored;
  std::vector<Node> to_explore;

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

  while (!to_explore.empty())
  {
    Node node = std::move(to_explore.back());

    if (node.board.is_equal(target))
      return { true, node.path };

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

  return { false, { } };
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
      to_explore.push({ 0, source, { } });

      while (!to_explore.empty())
      {
        Node node = std::move(to_explore.top());
        to_explore.pop();

        if (node.board.is_equal(target))
          return { true, std::move(node.path) };
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

      return { false, { } };
    };

  for (size_t depth = 0; depth <= max_depth; depth++)
  {
    auto res = iterative_deepening_search_aux(depth);

    if (res.found_path)
      return res;
  }

  return { false, { } };
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

  while (!to_explore.empty())
  {
    Node node = std::move(to_explore.front());
    to_explore.pop();

    if (node.board.is_equal(target))
      return { true, std::move(node.path) };

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

  return { false, { } };
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

  while (!to_explore.empty())
  {
    Node node = std::move(*to_explore.begin());
    to_explore.erase(to_explore.begin());

    if (node.board.is_equal(target))
      return { true, std::move(node.path) };

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

  return { false, { } };
}

SearchResult greedy_search_with_heuristics(
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

  while (!to_explore.empty())
  {
    Node node = std::move(*to_explore.begin());
    to_explore.erase(to_explore.begin());

    if (node.board.is_equal(target))
      return { true, std::move(node.path) };

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

  return { false, { } };
}

int main()
{
  Board source = { 0, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 15, 10, 11, 12, 9, 13, 14 }};
  Board target = { 15, { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 }};

  source.set_pos();
  target.set_pos();

  std::cout << source.can_be_solved(target) <<
    '\n' << source.is_equal(target) << '\n';

  auto res = a_star_search(source, target, sum_of_manhattan_distances);

  if (res.found_path)
  {
    size_t i = 0;
    for (auto &dir: res.path)
    {
      i++;
      std::cout << to_string(dir) << (i % 8 == 0 ? '\n' : ' ');
    }
  }

  for (int i = 0; i < Direction::COUNT; i++)
    std::cout << source.can_move((Direction)i) << '\n';
}
