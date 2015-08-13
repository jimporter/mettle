CXXFLAGS := -std=c++1y
PREFIX := /usr

-include config.mk

ifdef MKDOCS_VENV
  MKDOCS := . $(MKDOCS_VENV)/bin/activate && mkdocs
else
  MKDOCS := mkdocs
endif

NON_TEST_DIRS := test/windows
TEST_DIRS := $(filter-out $(NON_TEST_DIRS),$(shell find test -type d))
TESTS := $(patsubst %.cpp,%,$(foreach d,$(TEST_DIRS),$(wildcard $(d)/*.cpp)))
TEST_DATA := $(patsubst %.cpp,%,$(wildcard test_data/*.cpp))

EXAMPLES := $(patsubst %.cpp,%,$(wildcard examples/*.cpp))
HEADER_ONLY_EXAMPLES := examples/test_header_only
LIB_EXAMPLES := $(filter-out $(HEADER_ONLY_EXAMPLES),$(EXAMPLES))

METTLE_DIRS := src/mettle src/mettle/posix
METTLE_SOURCES := $(foreach dir,$(METTLE_DIRS),$(wildcard $(dir)/*.cpp))
LIBMETTLE_DIRS := src/libmettle src/libmettle/log src/libmettle/posix
LIBMETTLE_SOURCES := $(foreach dir,$(LIBMETTLE_DIRS),$(wildcard $(dir)/*.cpp))
SOURCES := $(METTLE_SOURCES) $(LIBMETTLE_SOURCES)
LIBS := -lboost_program_options -lboost_iostreams -pthread

all: mettle libmettle.so

# Include all the existing dependency files for automatic #include dependency
# handling.
-include $(TESTS:=.d)
-include $(EXAMPLES:=.d)
-include $(SOURCES:.cpp=.d)

# Build .o files and the corresponding .d (dependency) files. For more info, see
# <http://scottmcpeak.com/autodepend/autodepend.html>.
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -Iinclude -MMD -MF $*.d -c $< -o $@
	@sed -e 's/.*://' -e 's/\\$$//' < $*.d | fmt -1 | \
	  sed -e 's/^ *//' -e 's/$$/:/' >> $*.d

TEST_LDFLAGS := $(LDFLAGS)
test/driver/test_cmd_line test/driver/test_test_file \
test/driver/test_run_test_files: \
  TEST_LDFLAGS += -lboost_program_options
test/driver/test_run_test_files: TEST_LDFLAGS += -lboost_iostreams

test/driver/test_test_file: src/test_file.o
test/driver/test_run_test_files: \
  src/posix/run_test_file.o src/run_test_files.o src/test_file.o

$(TESTS) $(TEST_DATA) $(LIB_EXAMPLES): %: %.o libmettle.so
	$(CXX) $(CXXFLAGS) $(filter %.o,$^) -L. -lmettle $(TEST_LDFLAGS) -o $@

$(HEADER_ONLY_EXAMPLES): %: %.o
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

.PHONY: examples
examples: $(EXAMPLES)

.PHONY: tests
tests: $(TESTS)

.PHONY: test-data
test-data: $(TEST_DATA)

mettle: MY_LDFLAGS := $(LDFLAGS) -lboost_program_options -lboost_iostreams
mettle: $(METTLE_SOURCES:.cpp=.o) libmettle.so
	$(CXX) $(CXXFLAGS) $(filter %.o,$^) -L. -lmettle $(MY_LDFLAGS) -o $@

libmettle.so: CXXFLAGS += -fPIC
libmettle.so: MY_LDFLAGS := $(LDFLAGS) $(LIBS)
libmettle.so: $(LIBMETTLE_SOURCES:.cpp=.o)
	$(CXX) -shared $(CXXFLAGS) $^ -L. $(MY_LDFLAGS) -o $@

.PHONY: install
install: all
	cp -R include $(PREFIX)
	cp mettle $(PREFIX)/bin/mettle
	cp libmettle.so $(PREFIX)/lib/libmettle.so

.PHONY: test
test: mettle tests test-data
	$(eval DATA_DIR := $(shell readlink -f test/test_data))
	TEST_DATA=$(DATA_DIR)/ ./mettle --output=verbose --color=auto $(TESTS)

.PHONY: doc
doc:
	$(MKDOCS) build --clean

.PHONY: doc-serve
doc-serve:
	$(MKDOCS) serve --dev-addr=0.0.0.0:8000

.PHONY: doc-deploy
doc-deploy:
	$(MKDOCS) gh-deploy --clean

.PHONY: clean
clean: clean-tests clean-examples clean-bin clean-obj

.PHONY: clean-tests
clean-tests:
	rm -f $(TESTS) $(TEST_DATA)

.PHONY: clean-examples
clean-examples:
	rm -f $(EXAMPLES)

.PHONY: clean-bin
clean-bin:
	rm -f mettle libmettle.so

.PHONY: clean-obj
clean-obj:
	find . -name "*.[od]" -exec rm -f {} +

.PHONY: gitignore
gitignore:
	@echo $(TESTS) $(TEST_DATA) | sed -e 's|test/||g' -e 's/ /\n/g' > \
          test/.gitignore
	@echo $(EXAMPLES) | sed -e 's|examples/||g' -e 's/ /\n/g' > \
	  examples/.gitignore
