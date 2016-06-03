/*
    Copyright (c) 2016 Vladimir Michl

    This file is part of Exception Stack Trace library.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "exception_stack_trace.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <sstream>
#include <typeinfo>
#include <stdint.h>

#define STACK_ENTRIES_MAX 64
#define STACK_CONCURRENT_MAX 12

namespace {
  void *(*real_cxa_allocate_exception)(size_t size) = 0;
  void *(*real_cxa_free_exception)(void *exception) = 0;
  void (*real_cxa_throw)(void *exception, struct std::type_info * tinfo, void (*dest)(void *)) = 0;

  void *get_real_function(const char *function) {
    void *real = dlsym(RTLD_NEXT, function);
    if (!real) {
      fprintf(stderr, "ERROR: cannot find libstdc++.%s: %s\n", function, dlerror());
      std::terminate();
    }
    return real;
  };

  __attribute__((constructor)) void init() {
    printf("init called\n");
    real_cxa_allocate_exception = (void *(*)(size_t))get_real_function("__cxa_allocate_exception");
    real_cxa_free_exception = (void *(*)(void *))get_real_function("__cxa_free_exception");
    real_cxa_throw = (void (*)(void *exception, struct std::type_info * tinfo, void (*dest)(void *)))get_real_function("__cxa_throw");
  }
  
  struct StackTrace {
    int size;
    const char *name;
    void *stack[STACK_ENTRIES_MAX];
  };

  struct ExceptionData {
    void *exception;
    StackTrace *stack_trace;
  };
    
  __thread int index=-1;
  __thread ExceptionData exception_data[STACK_CONCURRENT_MAX];
}

extern "C" {
  void * __cxa_allocate_exception(size_t size) {
    void *ret=0;
    printf("exception allocated\n");
    ret = real_cxa_allocate_exception(size+sizeof(StackTrace));
    void *stack=static_cast<uint8_t*>(ret)+size;
    printf("exception allocated raw: %p\n", ret);
    ++index;
    exception_data[index].exception=ret;
    exception_data[index].stack_trace = static_cast<StackTrace *>(stack);
    printf("exception allocated ret: %p\n", ret);
    return ret;
  }
  void __cxa_free_exception(void *exception) {
    printf("exception free in: %p\n", exception);
    --index;
    real_cxa_free_exception(exception);
  }

  void __cxa_throw(void *exception, struct std::type_info * tinfo, void (*dest)(void *)) {
    printf("throw in: %p\n", exception);
    StackTrace &stack=*exception_data[index].stack_trace;
    stack.size = backtrace(stack.stack, STACK_ENTRIES_MAX);
    stack.name = tinfo->name();
    real_cxa_throw(exception, tinfo, dest);
  };
}

namespace exceptionstacktrace {
  int get_stack_trace_raw(void * const *&stacktrace, const char *&name, const void *exception) {
    printf("stack trace raw: %p\n", exception);
    const StackTrace *stack=0;
    for (int i=index; i >=0; --i) {
      if (exception_data[i].exception == exception) {
	stack = exception_data[i].stack_trace;
	break;
      }
    }
    printf("stack trace: %p\n", stack);
    stacktrace = stack->stack;
    name = stack->name;
    return stack->size;
  }

  std::string get_stack_trace_names(const void *exception) {
    std::ostringstream os;
    void *const *stack;
    const char *name;
    int size=get_stack_trace_raw(stack, name, exception);
    char **text=backtrace_symbols(stack, size);
    os << name << std::endl;
    for (int i = 0; i < size; ++i) {
      os << text[i] << std::endl;
    }
    free(text);
    return os.str();
  }

  std::string get_stack_trace_names(const std::exception &exception) {
    return get_stack_trace_names(&exception);
  }
}
