#!/bin/bash

g++ -std=c++11 MyBot.cpp -o MyBot.o
g++ -std=c++11 RandomBot.cpp -o RandomBot.o
g++ -std=c++11 HelineBot.cpp -o HelineBot.o

./halite -q -d "30 30" "./HelineBot.o" "./MyBot.o"
