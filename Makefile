CXX := clang++
CXXFLAGS := -std=c++1y -stdlib=libc++ -Wall -Wextra -pedantic -Werror
LDFLAGS := -lsupc++

TESTS := $(patsubst %.cpp,%,$(wildcard test/*.cpp))
EXAMPLES := $(patsubst %.cpp,%,$(wildcard examples/*.cpp))
SOURCES := $(wildcard src/*.cpp src/libmettle/*.cpp)

# Include all the existing dependency files for automatic #include dependency
# handling.
-include $(TESTS:=.d)
-include $(EXAMPLES:=.d)
-include $(SOURCES:.cpp=.d)

all: mettle libmettle.so

# Build .o files and the corresponding .d (dependency) files. For more info, see
# <http://scottmcpeak.com/autodepend/autodepend.html>.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -Iinclude -c $< -o $@
	$(eval TEMP := $(shell mktemp))
	@$(CXX) $(CXXFLAGS) -MM -Iinclude $< > $(TEMP)
	@sed -e 's|.*:|$*.o:|' < $(TEMP) > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $(TEMP) | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $(TEMP)

TEST_LDFLAGS := $(LDFLAGS)
test/test_child: TEST_LDFLAGS += -lboost_iostreams
test/test_child: src/file_runner.o

$(TESTS) $(EXAMPLES): %: %.o libmettle.so
	$(CXX) $(CXXFLAGS) $(filter %.o,$^) -L. -lmettle $(TEST_LDFLAGS) -o $@

examples: $(EXAMPLES)

tests: $(TESTS)

SRC_LDFLAGS := $(LDFLAGS)
mettle: SRC_LDFLAGS += -lboost_program_options -lboost_iostreams
mettle: src/mettle.o src/file_runner.o
	$(CXX) $(CXXFLAGS) $^ $(SRC_LDFLAGS) -o $@

libmettle.so: CXXFLAGS += -fPIC
libmettle.so: SRC_LDFLAGS += -lboost_program_options
libmettle.so: src/libmettle/driver.o
	$(CXX) -shared $(CXXFLAGS) $< $(SRC_LDFLAGS) -o $@

.PHONY: test
test: tests mettle
	./mettle --verbose 2 --color $(TESTS)

.PHONY: clean
clean: clean-tests clean-examples clean-src

.PHONY: clean-tests
clean-tests:
	rm -f $(TESTS) test/*.[od]

.PHONY: clean-examples
clean-examples:
	rm -f $(EXAMPLES) examples/*.[od]

.PHONY: clean-src
clean-src:
	rm -f mettle libmettle.so src/*.[od] src/libmettle/*.[od]

.PHONY: gitignore
gitignore:
	@echo $(TESTS) | sed -e 's|test/||g' -e 's/ /\n/g' > test/.gitignore
	@echo $(EXAMPLES) | sed -e 's|examples/||g' -e 's/ /\n/g' > \
	  examples/.gitignore
