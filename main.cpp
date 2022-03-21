#include <iostream>
#include <vector>
#include <fstream>
#include <cassert>

#include "src/Enums.hpp"
#include "src/SearchAlgorithms.hpp"

std::vector<Board> read_boards_from_file(char const *filepath)
{
  std::fstream file(filepath, std::fstream::in);

  if (!file.is_open())
  {
    std::cerr << "error: couldn't open the file \'"
              << filepath << "\'\n";
    exit(EXIT_FAILURE);
  }

  std::vector<Board> res;

  Board board;
  while (!file.eof())
  {
    size_t i = 0;
    for (unsigned piece = 0; i < 4 * 4 && file >> piece; i++)
    {
      if (piece > 15)
      {
        std::cerr << "error: incorrect value at position "
                  << i << " provided: " << piece << '\n';
        exit(EXIT_FAILURE);
      }

      piece = piece > 0 ? piece - 1 : 15;
      board.data[i] = piece;
      if (piece == 15)
        board.pos = i;
    }

    if (i == 4 * 4)
      res.push_back(board);
    else if (i != 0)
      std::cerr << "warning: some pieces are missing, skipping the board...\n";
  }

  file.close();

  return res;
}

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    std::cerr << "usage: " << argv[0] << " [source boards file] [targets boards file]\n";

    return EXIT_FAILURE;
  }

  Board const default_target = {
    15, {  0,  1,  2,  3,
           4,  5,  6,  7,
           8,  9, 10, 11,
          12, 13, 14, 15 }
  };
  auto const sources = read_boards_from_file(argv[1]);
  auto targets = read_boards_from_file(argv[2]);

  if (targets.size() < sources.size())
  {
    for (size_t diff = sources.size() - targets.size(); diff-- > 0; )
      targets.push_back(default_target);
  }

  for (size_t i = 0; i < sources.size(); i++)
  {
    std::cout << "--------------------- Board no. "
              << (i + 1)
              << " ---------------------\n";

    for (int j = 0; j < ALGORITHM_COUNT; j++)
    {
      SearchAlgorithm algorithm = (SearchAlgorithm)j;

      std::cout << "Algorithm: " << to_string(algorithm) << '\n';

      auto const res =
        choose_algorithm(algorithm, sources[i], targets[i]);

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

        std::cout << "\n   - Spent          "
                  << res.time_spent.count() << " seconds\n"
                  << "   - Took           "
                  << res.path.size() << " moves\n"
                  << "   - Explored       "
                  << res.explored_nodes_count << " nodes in total\n"
                  << "   - Stored at most "
                  << res.max_nodes_in_stack <<  " nodes in stack\n";
      }
    }

    std::cout << '\n';
  }
}
