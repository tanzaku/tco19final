#!/bin/bash

set -eu

g++ MultiplayerChessPieces.cpp -O3 -std=c++11 -Wall -Wextra -Wshadow

java -jar tester.jar -exec "./a.out" -seed 2 -size 15

# for i in $(seq 1 10); do
#     java -jar tester.jar -exec "./a.out" -seed ${i} -novis | grep "Score = "
# done
