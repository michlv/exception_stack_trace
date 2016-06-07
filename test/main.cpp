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

#include <iostream>
#include <stdexcept>
#include <vector>
#include "../include/exception_stack_trace.h"

#define EXPECT_EQ(aExpected, aActual) \
  { \
    std::string expected=aExpected; \
    std::string actual=aActual;	    \
    if (expected != actual) {			\
      std::cout << "Failed!";						\
      std::cout << "Expected: " << std::endl << "'" << expected << "'" << std::endl; \
      std::cout << "Actual: " << std::endl << "'" <<  actual << "'" << std::endl; \
      return 1;								\
    }									\
  }

using namespace exceptionstacktrace;

struct MyException : public std::runtime_error {
  int x;
  int y;
  MyException(const std::string &msg) : std::runtime_error(msg), x(1), y(2) {
  };
};

#define THROW_CODE_TEST(name, throw_code, catch_exception, expected_simple_stack) \
int name() { \
  try { \
  std::cout << std::endl << "=================" << std::endl; \
    std::cout << #name << ": about to throw" << std::endl;	\
    throw_code ;						\
  } catch (const catch_exception &e) { \
    std::cout << #catch_exception << "(" << #throw_code << ") caught: " << &e << std::endl; \
    std::cout << "stack: " << std::endl << get_stack_trace_names(e); \
    std::string simpleStack = getStackTrace(e)->getSimpleStackTrace(); \
    std::cout << "stackSimple: " << std::endl << simpleStack;	       \
    EXPECT_EQ(expected_simple_stack, simpleStack);	      \
  } catch (...) { \
    std::cout << "... caught" << std::endl; \
    return 1; \
  } \
  std::cout << std::endl; \
  return 0; \
}

#define THROW_TEST(name, throw_exception, catch_exception, expected_simple_stack) \
  THROW_CODE_TEST(name, throw throw_exception, catch_exception, expected_simple_stack)

THROW_TEST(testRuntimeError, std::runtime_error("Hello"), std::exception, "\
std::runtime_error\n\
# 0 testRuntimeError(){./main}\n\
# 1 main{./main}\n\
# 2 __libc_start_main{/lib/x86_64-linux-gnu/libc.so.6}\n\
");
THROW_TEST(testRuntimeError1, std::runtime_error("Hello"), std::runtime_error, "\
std::runtime_error\n\
# 0 testRuntimeError1(){./main}\n\
# 1 main{./main}\n\
# 2 __libc_start_main{/lib/x86_64-linux-gnu/libc.so.6}\n\
");
THROW_TEST(testMyException, MyException("Hello"), std::exception, "\
MyException\n\
# 0 testMyException(){./main}\n\
# 1 main{./main}\n\
# 2 __libc_start_main{/lib/x86_64-linux-gnu/libc.so.6}\n\
");
THROW_TEST(testMyException1, MyException("Hello"), MyException, "\
MyException\n\
# 0 testMyException1(){./main}\n\
# 1 main{./main}\n\
# 2 __libc_start_main{/lib/x86_64-linux-gnu/libc.so.6}\n\
");

THROW_CODE_TEST(testStdVector, std::vector<int> v; v.at(0), std::exception,"\
std::out_of_range\n\
# 0 std::__throw_out_of_range_fmt(char const*, ...){/usr/lib/x86_64-linux-gnu/libstdc++.so.6}\n\
# 1 std::vector<int, std::allocator<int> >::_M_range_check(unsigned long) const{./main}\n\
# 2 std::vector<int, std::allocator<int> >::at(unsigned long){./main}\n\
# 3 testStdVector(){./main}\n\
# 4 main{./main}\n\
# 5 __libc_start_main{/lib/x86_64-linux-gnu/libc.so.6}\n\
");

class InnerException: public std::exception {
};

class OutterException: public std::exception {
};

std::string expectedOuterException="\
OutterException\n\
# 0 testDoubleException(){./main}\n\
# 1 main{./main}\n\
# 2 __libc_start_main{/lib/x86_64-linux-gnu/libc.so.6}\n\
";

int testDoubleExceptionInner(const std::exception &outer) {
  try {
    throw InnerException();
  } catch (const std::exception &e) {
    std::string simpleStack=getStackTrace(e)->getSimpleStackTrace();
    std::cout << simpleStack << std::endl;
    EXPECT_EQ("\
InnerException\n\
# 0 testDoubleExceptionInner(std::exception const&){./main}\n\
# 1 testDoubleException(){./main}\n\
# 2 main{./main}\n\
# 3 __libc_start_main{/lib/x86_64-linux-gnu/libc.so.6}\n\
", simpleStack);
    simpleStack=getStackTrace(outer)->getSimpleStackTrace();
    std::cout << simpleStack << std::endl;      
    EXPECT_EQ(expectedOuterException, simpleStack);
  }
  return 0;
};

int testDoubleException() {
  try {
    throw OutterException();
  } catch (const std::exception &e) {
    std::string simpleStack=getStackTrace(e)->getSimpleStackTrace();
    std::cout << simpleStack << std::endl;
    EXPECT_EQ(expectedOuterException, simpleStack);
    return testDoubleExceptionInner(e);
  }
  return 0;
};

	       
int main() {
  std::cout << "main" << std::endl;
  int error=0;
  error += testRuntimeError();
  error += testRuntimeError1();
  error += testMyException();
  error += testMyException1();
  error += testStdVector();
  error += testDoubleException();
  std::cout << "about to return" << std::endl;
  return error;
}
