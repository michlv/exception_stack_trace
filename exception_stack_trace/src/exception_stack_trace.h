#include <string>
#include <stdexcept>

namespace exceptionstacktrace {
  int get_stack_trace_raw(void * const *&stacktrace, const char *&name, const void *exception);
  std::string get_stack_trace_names(const void *exception);
  std::string get_stack_trace_names(const std::exception &exception);
}
