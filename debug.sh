#!/bin/bash

set -eu

g++ MultiplayerChessPieces.cpp -std=c++11 -fsanitize=address -fsanitize=undefined -Wall -Wextra -Wshadow
java -jar tester.jar -exec "./a.out" -seed 1
