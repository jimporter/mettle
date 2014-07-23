CXX := clang++
CXXFLAGS := -std=c++1y -stdlib=libc++ -Wall -Wextra -pedantic -Werror
LDFLAGS := -lboost_program_options -lsupc++

TESTS := $(patsubst %.cpp,%,$(wildcard test/*.cpp))
EXAMPLES := $(patsubst %.cpp,%,$(wildcard examples/*.cpp))

# Include all the existing dependency files for automatic #include dependency
# handling.
-include $(TESTS:=.d)
-include $(EXAMPLES:=.d)
-include src/mettle.d

all: mettle

test/test_child: LDFLAGS += -lboost_iostreams

# Build .o files and the corresponding .d (dependency) files. For more info, see
# <http://scottmcpeak.com/autodepend/autodepend.html>.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -Iinclude -c $< -o $@
	$(eval TEMP := $(shell mktemp))
	$(CXX) $(CXXFLAGS) -MM -Iinclude $< > $(TEMP)
	@sed -e 's|.*:|$*.o:|' < $(TEMP) > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $(TEMP) | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $(TEMP)

$(TESTS) $(EXAMPLES): %: %.o
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

examples: $(EXAMPLES)

tests: $(TESTS)

mettle: LDFLAGS += -lboost_iostreams
mettle: src/mettle.o
	$(CXX) $(CXXFLAGS) $< $(LDFLAGS) -o $@

.PHONY: test
test: tests mettle
	./mettle --verbose 2 --color $(TESTS)

.PHONY: clean
clean: clean-tests clean-examples clean-mettle

.PHONY: clean-tests
clean-tests:
	rm -f $(TESTS) test/*.o test/*.d

.PHONY: clean-examples
clean-examples:
	rm -f $(EXAMPLES) examples/*.o examples/*.d

.PHONY: clean-mettle
clean-mettle:
	rm -f mettle src/*.o src/*.d

.PHONY: gitignore
gitignore:
	@echo $(TESTS) | sed -e 's|test/||g' -e 's/ /\n/g' > test/.gitignore
	@echo $(EXAMPLES) | sed -e 's|examples/||g' -e 's/ /\n/g' > \
	  examples/.gitignore
