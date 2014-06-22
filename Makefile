CXX := clang++
CXXFLAGS := -std=c++1y -stdlib=libc++ -Wall -Wextra -pedantic -Werror
LDFLAGS := -lboost_program_options -lsupc++

TESTS := $(patsubst %.cpp,%,$(wildcard test/*.cpp))
EXAMPLES := $(patsubst %.cpp,%,$(wildcard examples/*.cpp))

# Include all the existing dependency files for automatic #include dependency
# handling.
-include $(TESTS:=.d)
-include $(EXAMPLES:=.d)

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

.PHONY: test
test: test/test_all
	test/test_all --verbose 2 --color

.PHONY: clean
clean: clean-tests clean-examples

.PHONY: clean-tests
clean-tests:
	rm -f $(TESTS) test/*.o test/*.d

.PHONY: clean-examples
clean-examples:
	rm -f $(EXAMPLES) examples/*.o examples/*.d

.PHONY: gitignore
gitignore:
	@echo $(TESTS) | sed -e 's|test/||g' -e 's/ /\n/g' > test/.gitignore
	@echo $(EXAMPLES) | sed -e 's|examples/||g' -e 's/ /\n/g' > \
	  examples/.gitignore
