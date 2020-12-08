#!/bin/bash
g++ img-par.cpp -o test -std=c++17 -Wall -Wextra -Wno-deprecated -Werror -pedantic -pedantic-errors -fopenmp -O3 -DNDEBUG
for i in {1..5}
do
	./test sobel test-images/ output-test/ > outputs/16_threads_$i.txt
done
