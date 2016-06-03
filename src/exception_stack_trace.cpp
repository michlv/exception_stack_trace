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
#include <cxxabi.h>
#include <new>

#define STACK_ENTRIES_MAX 64
#define STACK_CONCURRENT_MAX 12

#if 0
# define DEBUG(x) x
#else
# define DEBUG(x)
#endif

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
    DEBUG(printf("init called\n"));
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
    
  __thread int exception_data_index=-1;
  __thread ExceptionData exception_data[STACK_CONCURRENT_MAX];

  char *demangle(const char *mangled_name, char *output_buffer, size_t *length, int *status) {
    char *ret = abi::__cxa_demangle(mangled_name, output_buffer, length, status);
    if (!ret)
      return output_buffer;
    return ret;
  }
}

extern "C" {
  void * __cxa_allocate_exception(size_t size) {
    void *ret=0;
    DEBUG(printf("exception allocated\n"));
    ret = real_cxa_allocate_exception(size+sizeof(StackTrace));
    void *stack=static_cast<uint8_t*>(ret)+size;
    DEBUG(printf("exception allocated raw: %p\n", ret));
    ++exception_data_index;
    exception_data[exception_data_index].exception=ret;
    exception_data[exception_data_index].stack_trace = static_cast<StackTrace *>(stack);
    DEBUG(printf("exception allocated ret: %p\n", ret));
    return ret;
  }
  void __cxa_free_exception(void *exception) {
    DEBUG(printf("exception free in: %p\n", exception));
    --exception_data_index;
    real_cxa_free_exception(exception);
  }

  void __cxa_throw(void *exception, struct std::type_info * tinfo, void (*dest)(void *)) {
    DEBUG(printf("throw in: %p\n", exception));
    StackTrace &stack=*exception_data[exception_data_index].stack_trace;
    stack.size = backtrace(stack.stack, STACK_ENTRIES_MAX);
    stack.name = tinfo->name();
    real_cxa_throw(exception, tinfo, dest);
  };
}

namespace exceptionstacktrace {
  int get_stack_trace_raw(void * const *&stacktrace, const char *&name, const void *exception) {
    DEBUG(printf("stack trace raw: %p\n", exception));
    const StackTrace *stack=0;
    for (int i=exception_data_index; i >=0; --i) {
      if (exception_data[i].exception == exception) {
	stack = exception_data[i].stack_trace;
	break;
      }
    }
    DEBUG(printf("stack trace: %p\n", stack));
    stacktrace = stack->stack;
    name = stack->name;
    return stack->size;
  }

  std::string get_stack_trace_names(const void *exception) {
    std::ostringstream os;
    void *const *stack;
    const char *name;
    int size=get_stack_trace_raw(stack, name, exception);
    size_t bsize=64;
    char *buffer=static_cast<char*>(malloc(bsize));
    if (!buffer)
      return "";
    char **text=backtrace_symbols(stack, size);
    int status;
    buffer = demangle(name, buffer, &bsize, &status);
    os << (status==0?buffer:name) << std::endl;
    for (int i = stack_trace_raw_start_index; i < size; ++i) {
      std::string str(text[i]);
      size_t s=str.find_first_of('(');
      size_t e=str.find_last_of('+');
      if (s!=std::string::npos && e!=std::string::npos) {
	std::string n = str.substr(s+1, e-s-1);
	buffer=demangle(n.c_str(), buffer, &bsize, &status);
	if (status == 0) {
	  str.replace(s+1, e-s-1, buffer);
	}
      }
      os << str << std::endl;
    }
    free(text);
    free(buffer);
    return os.str();
  }

  std::string get_stack_trace_names(const std::exception &exception) {
    return get_stack_trace_names(&exception);
  }
}
