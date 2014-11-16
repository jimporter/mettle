CXXFLAGS := -std=c++1y
PREFIX := /usr

-include config.mk

ifdef MKDOCS_VENV
  MKDOCS := . $(MKDOCS_VENV)/bin/activate && mkdocs
else
  MKDOCS := mkdocs
endif

ifndef TMPDIR
  TMPDIR := /tmp
endif

TESTS := $(patsubst %.cpp,%,$(wildcard test/*.cpp))
EXAMPLES := $(patsubst %.cpp,%,$(wildcard examples/*.cpp))
HEADER_ONLY_EXAMPLES := examples/test_header_only

METTLE_SOURCES := $(wildcard src/*.cpp)
LIBMETTLE_SOURCES := $(shell find src/libmettle -type f -name "*.cpp")
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
	$(eval TEMP := $(shell mktemp $(TMPDIR)/mettle-XXXXXX))
	@$(CXX) $(CXXFLAGS) -MM -Iinclude $< > $(TEMP)
	@sed -e 's|.*:|$*.o:|' < $(TEMP) > $*.d
	@sed -e 's/.*://' -e 's/\\$$//' < $(TEMP) | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d
	@rm -f $(TEMP)

TEST_LDFLAGS := $(LDFLAGS)
test/test_child: TEST_LDFLAGS += -lboost_program_options -lboost_iostreams
test/test_child: src/run_test_files.o

$(TESTS) $(filter-out $(HEADER_ONLY_EXAMPLES),$(EXAMPLES)): %: %.o libmettle.so
	$(CXX) $(CXXFLAGS) $(filter %.o,$^) -L. -lmettle $(TEST_LDFLAGS) -o $@

$(HEADER_ONLY_EXAMPLES): %: %.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

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
	./mettle --output=verbose --color $(TESTS)

.PHONY: doc
doc:
	$(MKDOCS) build --clean

.PHONY: serve-doc
serve-doc:
	$(MKDOCS) serve --dev-addr=0.0.0.0:8000

.PHONY: deploy-doc
deploy-doc:
	$(MKDOCS) gh-deploy

.PHONY: clean
clean: clean-tests clean-examples clean-bin clean-obj

.PHONY: clean-tests
clean-tests:
	rm -f $(TESTS)

.PHONY: clean-examples
clean-examples:
	rm -f $(EXAMPLES)

.PHONY: clean-bin
clean-src:
	rm -f mettle libmettle.so

.PHONY: clean-obj
clean-obj:
	find . -name "*.[od]" -exec rm -f {} +

.PHONY: gitignore
gitignore:
	@echo $(TESTS) | sed -e 's|test/||g' -e 's/ /\n/g' > test/.gitignore
	@echo $(EXAMPLES) | sed -e 's|examples/||g' -e 's/ /\n/g' > \
	  examples/.gitignore
