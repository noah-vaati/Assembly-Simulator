all: start

start: start.cpp
	clang++ -o start start.cpp