#ifndef SEARCH_ALGORITHMS_HPP
#define SEARCH_ALGORITHMS_HPP

#include <list>
#include <chrono>

#include "Board.hpp"
#include "Enums.hpp"

#define MEMORY_LIMIT ((size_t)2 * 1024 * 1024 * 1024)
#define MAX_DEPTH 32

extern size_t memory_used;

template <class T>
struct CustomAllocator
{
  typedef ptrdiff_t difference_type;
  typedef const T*  const_pointer;
  typedef T&        reference;
  typedef const T&  const_reference;
  typedef T         value_type;

  T *allocate(size_t n, const void * = 0)
  {
    size_t const bytes = n * sizeof(T);
    memory_used += bytes;

    return (T*)new char[bytes];
  }

  void deallocate(void *p, size_t n)
  {
    delete[] (char *)p;
    memory_used -= n * sizeof(T);
  }
};

using Path = std::list<Direction, CustomAllocator<Direction>>;
using HeuristicFunc = size_t (*)(Board const &, Board const &);

struct SearchResult
{
  SearchStatus status;
  Path path;
  size_t explored_nodes_count;
  size_t max_nodes_in_stack;
  std::chrono::duration<double> time_spent;
};

SearchResult choose_algorithm(
  SearchAlgorithm algorithm, Board const &source, Board const &target
  );

#endif // SEARCH_ALGORITHMS_HPP
