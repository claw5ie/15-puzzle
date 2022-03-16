WFLAGS=-Wall -Wextra -pedantic
OFLAGS=-std=c++11 -g

all: main

main: ./main.cpp
	g++ $(WFLAGS) $(OFLAGS) -o main.out $^
