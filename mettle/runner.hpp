#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

int main() {
  for (auto s : mettle::suites)
    s();
}

#endif
