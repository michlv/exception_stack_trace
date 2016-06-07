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
#include <iomanip>

#define STACK_CONCURRENT_MAX 12

#if 0
# define DEBUG(x) x
#else
# define DEBUG(x)
#endif

namespace {
  class Demangler {
    size_t bsize;
    char *buffer;

  public:
    Demangler() : bsize(64) {
      buffer=static_cast<char*>(malloc(bsize));
    }
    ~Demangler() {
      free(buffer);
    }
    bool good() const {
      return buffer;
    }
    bool operator !() const {
      return !good();
    }
    
    const char *operator()(const char *mangled_name) {
      int status;
      char *ret = abi::__cxa_demangle(mangled_name, buffer, &bsize, &status);
      if (ret) {
	buffer=ret;
	return ret;
      }
      return mangled_name;
    }
  };

  struct SimpleStackOutputter : public info::StackTrace::WalkSymbols {
    std::ostream &os;
    int i;
    SimpleStackOutputter(std::ostream &aOs) : os(aOs), i(0) {};
    void operator()(const char *sname, void *saddr, void *addr, const char *fname) {
      os << "#" << std::setw(2) << std::setfill(' ') << i++ << " " << sname;
      if (fname) {
	os << "{" << fname << "}";
      }
      os << std::endl;
    }
  };
}

namespace info {
  StackTrace::StackTrace(const int suppressTopXSymbols, const char *aName)
    : suppress_top_x_symbols(suppressTopXSymbols), name(aName)
  {
    size = backtrace(stack, stack_size_max);
  };

  int StackTrace::getRaw(void * const *&stacktrace) const
  {
    DEBUG(printf("stack trace: %p\n", stack));
    stacktrace = &stack[suppress_top_x_symbols];
    return size-suppress_top_x_symbols;
  }
  const char *StackTrace::getName() const {
    return name;
  }

  std::string StackTrace::getSymbols() const {
    std::ostringstream os;
    void *const *stack;
    int size=getRaw(stack);
    Demangler demangler;
    if (!demangler)
      return "";
    char **text=backtrace_symbols(stack, size);
    if (name) {
      os << demangler(name) << std::endl;
    }
    for (int i = 0; i < size; ++i) {
      std::string str(text[i]);
      size_t s=str.find_first_of('(');
      size_t e=str.find_last_of('+');
      if (s!=std::string::npos && e!=std::string::npos) {
	std::string n = str.substr(s+1, e-s-1);
	str.replace(s+1, e-s-1, demangler(n.c_str()));
      }
      os << str << std::endl;
    }
    free(text);
    return os.str();
  }
  
  void StackTrace::walkSymbols(WalkSymbols &callBack) const {
    void *const *stack;
    int size=getRaw(stack);
    Dl_info dl_info;
    Demangler demangler;
    if (!demangler)
      return;
    for (int i = 0; i < size; ++i) {
      if (dladdr(stack[i], &dl_info) && dl_info.dli_sname != NULL) {
	callBack(demangler(dl_info.dli_sname),
		 dl_info.dli_saddr, stack[i], dl_info.dli_fname);
      }
    }
  }
  
  std::string StackTrace::getSimpleStackTrace() const {
    std::ostringstream os;
    Demangler demangler;
    if (!demangler)
      return "";
    if (name)
      os << demangler(name) << std::endl;
    SimpleStackOutputter o(os);
    walkSymbols(o);
    return os.str();
  }
}

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

  struct ExceptionData {
    void *exception;
    info::StackTrace *stack_trace;
  };
    
  __thread int exception_data_index=-1;
  __thread ExceptionData exception_data[STACK_CONCURRENT_MAX];

  void check_exception_data(void *exception) {
    if (exception_data_index < 0 || exception_data[exception_data_index].exception != exception) {
      fprintf(stderr, "__cxa_free_exception: index < 0 or out of order exception: %i %p", exception_data_index, exception);
      std::terminate();
    }
  };

  const info::StackTrace *findStackForException(const void *exception) {
    for (int i=exception_data_index; i >=0; --i) {
      if (exception_data[i].exception == exception) {
	return exception_data[i].stack_trace;
      }
    }
    return NULL;
  };
}

extern "C" {
  void * __cxa_allocate_exception(size_t size) {
    void *ret=0;
    DEBUG(printf("exception allocated\n"));
    ret = real_cxa_allocate_exception(size+sizeof(info::StackTrace));
    void *stack=static_cast<uint8_t*>(ret)+size;
    DEBUG(printf("exception allocated raw: %p\n", ret));
    ++exception_data_index;
    exception_data[exception_data_index].exception=ret;
    exception_data[exception_data_index].stack_trace = static_cast<info::StackTrace *>(stack);
    DEBUG(printf("exception allocated ret: %p\n", ret));
    return ret;
  }
  void __cxa_free_exception(void *exception) {
    DEBUG(printf("exception free in: %p\n", exception));
    check_exception_data(exception);
    --exception_data_index;
    real_cxa_free_exception(exception);
  }

  void __cxa_throw(void *exception, struct std::type_info * tinfo, void (*dest)(void *)) {
    DEBUG(printf("throw in: %p\n", exception));
    check_exception_data(exception);
    info::StackTrace *stack=exception_data[exception_data_index].stack_trace;
    stack = new (stack) info::StackTrace(2, tinfo->name());
    real_cxa_throw(exception, tinfo, dest);
  };
}

namespace exceptionstacktrace {
  const info::StackTrace *getStackTrace(const void *exception) {
    return findStackForException(exception);
  }
  const info::StackTrace *getStackTrace(const std::exception &exception) {
    return getStackTrace(&exception);
  }
  std::string get_stack_trace_names(const void *exception) {
    const info::StackTrace *stack = findStackForException(exception);
    if (!stack)
      return "";
    return stack->getSymbols();
  }

  std::string get_stack_trace_names(const std::exception &exception) {
    return get_stack_trace_names(&exception);
  }
}
