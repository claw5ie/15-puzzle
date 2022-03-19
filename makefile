WFLAGS=-Wall -Wextra -pedantic
OFLAGS=-std=c++11 -g
OUT=search.out

all: debug_build

debug_build: ./main.cpp ./src/Board.cpp ./src/Enums.cpp ./src/SearchAlgorithms.cpp
	g++ $(WFLAGS) $(OFLAGS) -o $(OUT) $^

release_build: ./main.cpp ./src/Board.cpp ./src/Enums.cpp ./src/SearchAlgorithms.cpp
	g++ -O3 -o $(OUT) $^
