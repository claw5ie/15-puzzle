#include <iostream>
#include <cassert>

#include "src/Enums.hpp"
#include "src/SearchAlgorithms.hpp"

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
    std::cout << "*********************************************\n";

    for (int j = 0; j < ALGORITHM_COUNT; j++)
    {
      SearchAlgorithm algorithm = (SearchAlgorithm)j;

      std::cout << "Algorithm: " << to_string(algorithm) << '\n';

      auto res = choose_algorithm(algorithm, sources[i], target);

      std::cout << " * Status: " << to_string(res.status) << '\n';

      if (res.status != UNSOLVABLE)
      {
        std::cout << "   - Path:          ";

        {
          size_t i = 0;
          for (auto dir: res.path)
          {
            std::cout << to_string(dir);

            if (++i < res.path.size())
              std::cout << ' ';
          }
        }

        std::cout << "\n   - Spent          " << res.time_spent.count() << " seconds\n"
                  << "   - Took           " << res.path.size() << " moves\n"
                  << "   - Explored       " << res.explored_nodes_count << " nodes in total\n"
                  << "   - Stored at most " << res.max_nodes_in_stack <<  " nodes in stack\n";
      }
    }
  }
}
