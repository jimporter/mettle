.PHONY: clean test

test:
	clang++ -std=c++1y -Iinclude -lboost_program_options -Wall -Wextra -pedantic -o test/test_all test/test_all.cpp
	test/test_all --verbose --color

clean:
	rm test/test_all
