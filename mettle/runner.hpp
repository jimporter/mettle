#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

int main() {
  for (auto s : mettle::suites) {
    mettle::suite_results results = s();

    std::cout << results.suite_name << ": " << results.passes.size() << "/"
              << results.total_tests() << " tests passed";
    if (results.skips.size())
      std::cout << " (" << results.skips.size() << " skipped)";
    std::cout << std::endl;

    if (results.fails.size()) {
      for (auto i : results.fails) {
        std::cout << "  " << i.first << " FAILED: " << i.second << std::endl;
      }
    }
  }
}

#endif
