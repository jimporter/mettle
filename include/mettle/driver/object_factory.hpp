#ifndef INC_METTLE_DRIVER_OBJECT_FACTORY_HPP
#define INC_METTLE_DRIVER_OBJECT_FACTORY_HPP

#include <functional>
#include <map>
#include <stdexcept>
#include <string>

namespace mettle {

template<typename Result, typename ...Args>
class object_factory {
public:
  using result_type = std::unique_ptr<Result>;
  using function_type = std::function<result_type(Args...)>;
  using container_type = std::map<std::string, function_type>;
  using iterator = typename container_type::const_iterator;

  object_factory(std::string kind = "object") : object_kind_(std::move(kind)) {}

  void add(std::string name, function_type f) {
    registry_.emplace(std::move(name), std::move(f));
  }

  void set_default(std::string name) {
    assert(registry_.count(name) && "factory function not registered");
    default_logger_ = std::move(name);
  }

  result_type make(const std::string &name, Args... args) {
    std::string key = name.empty() ? default_logger_ : name;
    auto i = registry_.find(key);
    if(i == registry_.end()) {
      throw std::invalid_argument(
        "unknown " + object_kind_ + " \"" + name + "\""
      );
    }
    return i->second(args...);
  }

  iterator begin() const {
    return registry_.begin();
  }

  iterator end() const {
    return registry_.end();
  }
private:
  std::map<std::string, function_type> registry_;
  std::string object_kind_, default_logger_;
};

} // namespace mettle

#endif
