CXX := clang++
CXXFLAGS := -std=c++1y -stdlib=libc++ -Wall -Wextra -pedantic -Werror
LDFLAGS := -lboost_program_options -lsupc++

TESTS := $(patsubst %.cpp,%,$(wildcard test/*.cpp))
EXAMPLES := $(patsubst %.cpp,%,$(wildcard examples/*.cpp))

.PHONY: clean clean-tests clean-examples test examples

# Include all the existing dependency files for automatic #include dependency
# handling.
-include $(TESTS:=.d)
-include $(EXAMPLES:=.d)

test: test/test_all
	test/test_all --verbose 2 --color

examples: $(EXAMPLES)

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

clean: clean-tests clean-examples

clean-tests:
	rm -f $(TESTS) $(TESTS:=.o) $(TESTS:=.d)

clean-examples:
	rm -f $(EXAMPLES) $(EXAMPLES:=.o) $(EXAMPLES:=.d)

gitignore:
	@for file in $(TESTS) $(EXAMPLES) ; do \
	  grep -xq "$$file" .gitignore || echo $$file >> .gitignore ; \
	done
