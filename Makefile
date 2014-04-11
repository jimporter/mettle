.PHONY: clean test

test:
	clang++ -std=c++1y -stdlib=libc++ -Iinclude -Wall -Wextra -pedantic -Werror test/test_all.cpp -lboost_program_options -lsupc++ -o test/test_all
	test/test_all --verbose --color

clean:
	rm test/test_all
