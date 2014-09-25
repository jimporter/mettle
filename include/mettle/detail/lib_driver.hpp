#ifndef INC_METTLE_DETAIL_LIB_DRIVER_HPP
#define INC_METTLE_DETAIL_LIB_DRIVER_HPP

namespace mettle {

namespace detail {
  int real_main(int argc, const char *argv[]);
}

} // namespace mettle

int main(int argc, const char *argv[]) {
  return mettle::detail::real_main(argc, argv);
}

#endif
