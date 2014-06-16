CXXFLAGS := -std=c++1y -stdlib=libc++ -Wall -Wextra -pedantic -Werror

TESTS := $(patsubst %.cpp,%,$(wildcard test/*.cpp))
EXAMPLES := $(patsubst %.cpp,%,$(wildcard examples/*.cpp))

.PHONY: clean test

test: clean test/test_all
	test/test_all --verbose 2 --color

test/%: test/%.cpp
	clang++ $(CXXFLAGS) -Iinclude $< -lboost_program_options -lsupc++ -o $@

examples/%: examples/%.cpp
	clang++ $(CXXFLAGS) -Iinclude $< -lboost_program_options -lsupc++ -o $@

clean:
	rm -f $(EXAMPLES) $(TESTS)
