CXXFLAGS := -std=c++1y
PREFIX := /usr

-include config.mk

CXXFLAGS += -Wall -Wextra -pedantic -Werror

TESTS := $(patsubst %.cpp,%,$(wildcard test/*.cpp))
EXAMPLES := $(patsubst %.cpp,%,$(wildcard examples/*.cpp))

METTLE_SOURCES := $(wildcard src/*.cpp)
LIBMETTLE_SOURCES := $(wildcard src/libmettle/*.cpp)
SOURCES := $(METTLE_SOURCES) $(LIBMETTLE_SOURCES)

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

mettle: MY_LDFLAGS := $(LDFLAGS) -lboost_program_options -lboost_iostreams
mettle: $(METTLE_SOURCES:.cpp=.o) libmettle.so
	$(CXX) $(CXXFLAGS) $^ -L. -lmettle $(MY_LDFLAGS) -o $@

libmettle.so: CXXFLAGS += -fPIC
libmettle.so: MY_LDFLAGS := $(LDFLAGS) -lboost_program_options -lboost_iostreams
libmettle.so: $(LIBMETTLE_SOURCES:.cpp=.o)
	$(CXX) -shared $(CXXFLAGS) $^ -L. $(MY_LDFLAGS) -o $@

.PHONY: install
install: all
	cp -R include $(PREFIX)/include
	cp mettle $(PREFIX)/bin/mettle
	cp libmettle.so $(PREFIX)/lib/libmettle.so

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
