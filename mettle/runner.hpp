#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

int main() {
  size_t total_fails = 0;

  for(auto suite : mettle::suites) {
    std::vector<std::string> passes;
    std::vector<std::pair<std::string, std::string>> fails;
    std::vector<std::string> skips;

    for(auto test : *suite) {
      if(test.skip) {
        skips.push_back(test.name);
        continue;
      }

      auto result = test.function();
      if (result.passed)
        passes.push_back(test.name);
      else
        fails.push_back({ test.name, result.message });
    }

    std::cout << suite->name() << ": " << passes.size() << "/"
              << suite->size() << " tests passed";
    if (skips.size())
      std::cout << " (" << skips.size() << " skipped)";
    std::cout << std::endl;

    for (auto i : fails)
      std::cout << "  " << i.first << " FAILED: " << i.second << std::endl;

    total_fails += fails.size();
  }

  return total_fails;
}

#endif
