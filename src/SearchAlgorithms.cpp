#include <list>
#include <stack>
#include <queue>
#include <set>
#include <functional>
#include <cassert>

#include "SearchAlgorithms.hpp"

size_t memory_used = 0;
auto constexpr chrono_default_value = std::chrono::duration<double>();

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
      return { SOLVED, std::move(node.path), explored.size(), max_nodes_in_stack, chrono_default_value };

    if (to_explore.size() * sizeof(Node) + memory_used > MEMORY_LIMIT)
      return { MEMORY_EXCEEDED, { }, explored.size(), max_nodes_in_stack, chrono_default_value };

    explored.push_back(node.board);
    to_explore.pop_back();

    for (int i = 0; i < DIRECTION_COUNT; i++)
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

  return { NOT_SOLVED, { }, explored.size(), max_nodes_in_stack, chrono_default_value };
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
          return { SOLVED, std::move(node.path), explored_nodes_count, max_nodes_in_stack, chrono_default_value };

        if (node.depth > depth)
          continue;

        if (to_explore.size() * sizeof(Node) + memory_used > MEMORY_LIMIT)
          return { MEMORY_EXCEEDED, { }, explored_nodes_count, max_nodes_in_stack, chrono_default_value };

        for (int i = 0; i < DIRECTION_COUNT; i++)
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

      return { NOT_SOLVED, { }, explored_nodes_count, max_nodes_in_stack, chrono_default_value };
    };

  size_t max_nodes_in_stack = 0,
    explored_nodes_count = 0;

  SearchResult res;

  // Assuming that the stack always grows when the depth increases.
  for (size_t depth = 0; depth <= max_depth; depth++)
  {
    res = iterative_deepening_search_aux(depth);

    if (res.status == SOLVED)
      return res;
  }

  return { NOT_SOLVED, { }, max_nodes_in_stack, explored_nodes_count, chrono_default_value };
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
      return { SOLVED, std::move(node.path), max_nodes_in_stack, explored_nodes_count, chrono_default_value };

    if (to_explore.size() * sizeof(Node) + memory_used > MEMORY_LIMIT)
      return { MEMORY_EXCEEDED, { }, explored_nodes_count, max_nodes_in_stack, chrono_default_value };

    for (int i = 0; i < DIRECTION_COUNT; i++)
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

  return { NOT_SOLVED, { }, max_nodes_in_stack, explored_nodes_count, chrono_default_value };
}

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
      return { SOLVED, std::move(node.path), max_nodes_in_stack, explored_nodes_count, chrono_default_value };

    if (to_explore.size() * sizeof(Node) + memory_used > MEMORY_LIMIT)
      return { MEMORY_EXCEEDED, { }, explored_nodes_count, max_nodes_in_stack, chrono_default_value };

    for (int i = 0; i < DIRECTION_COUNT; i++)
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

  return { NOT_SOLVED, { }, max_nodes_in_stack, explored_nodes_count, chrono_default_value };
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
      return { SOLVED, std::move(node.path), max_nodes_in_stack, explored_nodes_count, chrono_default_value };

    if (to_explore.size() * sizeof(Node) + memory_used > MEMORY_LIMIT)
      return { MEMORY_EXCEEDED, { }, explored_nodes_count, max_nodes_in_stack, chrono_default_value };

    for (int i = 0; i < DIRECTION_COUNT; i++)
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

  return { NOT_SOLVED, { }, max_nodes_in_stack, explored_nodes_count, chrono_default_value };
}

SearchResult choose_algorithm(
  SearchAlgorithm algorithm, Board const &source, Board const &target
  )
{
  if (!source.can_be_solved(target))
    return { UNSOLVABLE, { }, 0, 0, chrono_default_value };

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
