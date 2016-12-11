#!/bin/bash

g++ -std=c++11 MyBot.cpp -o MyBot.o
# g++ -std=c++11 RandomBot.cpp -o RandomBot.o
# g++ -std=c++11 HelineBot.cpp -o HelineBot.o -g
g++ -std=c++11 ClusterBot.cpp -o ClusterBot.o

./halite -q -d "30 30" "./ClusterBot.o" "./MyBot.o"
