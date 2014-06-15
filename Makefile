CXXFLAGS := -std=c++1y -stdlib=libc++ -Wall -Wextra -pedantic -Werror

.PHONY: clean test

test:
	clang++ $(CXXFLAGS) -Iinclude test/test_all.cpp -lboost_program_options -lsupc++ -o test/test_all
	test/test_all --verbose 2 --color

examples/% : examples/%.cpp
	clang++ $(CXXFLAGS) -Iinclude $< -lboost_program_options -lsupc++ -o $@

clean:
	rm test/test_all
