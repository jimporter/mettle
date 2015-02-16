#ifndef INC_METTLE_DRIVER_OBJECT_FACTORY_HPP
#define INC_METTLE_DRIVER_OBJECT_FACTORY_HPP

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

namespace mettle {

template<typename Function>
class object_factory;

template<typename Result, typename ...Args>
class object_factory<Result(Args...)> {
public:
  using result_type = std::unique_ptr<Result>;
  using function_type = std::function<result_type(Args...)>;
  using container_type = std::map<std::string, function_type>;
  using iterator = typename container_type::const_iterator;

  void add(std::string name, function_type f) {
    registry_.emplace(std::move(name), std::move(f));
  }

  void alias(const std::string &name, std::string alias) {
    registry_.emplace(std::move(alias), registry_.at(name));
  }

  template<typename ...CallArgs>
  result_type make(const std::string &name, CallArgs &&...args) {
    return registry_.at(name)(std::forward<CallArgs>(args)...);
  }

  iterator begin() const {
    return registry_.begin();
  }

  iterator end() const {
    return registry_.end();
  }
private:
  container_type registry_;
};

} // namespace mettle

#endif
