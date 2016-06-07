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

#define EXPECT_EQ(expected, actual) \
  if (expected != actual) { \
    std::cout << "Failed!"; \
    std::cout << "Expected: " << std::endl << "'" << expected << "'" << std::endl; \
    std::cout << "Actual: " << std::endl << "'" <<  actual << "'" << std::endl;	\
    return 1; \
  } \


using namespace exceptionstacktrace;

struct MyException : public std::runtime_error {
  int x;
  int y;
  MyException(const std::string &msg) : std::runtime_error(msg), x(1), y(2) {
  };
};

#define THROW_TEST(name, throw_exception, catch_exception, expected_simple_stack)	\
int name() { \
  try { \
  std::cout << std::endl << "=================" << std::endl; \
    std::cout << #name << ": about to throw" << std::endl;	\
    throw throw_exception; \
  } catch (const catch_exception &e) { \
    std::cout << #catch_exception << "(" << #throw_exception << ") caught: " << &e << std::endl; \
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

THROW_TEST(testRuntimeError, std::runtime_error("Hello"), std::exception, "\
std::runtime_error\n\
# 0 testRuntimeError()(./main)\n\
# 1 main(./main)\n\
# 2 __libc_start_main(/lib/x86_64-linux-gnu/libc.so.6)\n\
");
THROW_TEST(testRuntimeError1, std::runtime_error("Hello"), std::runtime_error, "\
std::runtime_error\n\
# 0 testRuntimeError1()(./main)\n\
# 1 main(./main)\n\
# 2 __libc_start_main(/lib/x86_64-linux-gnu/libc.so.6)\n\
");
THROW_TEST(testMyException, MyException("Hello"), std::exception, "\
MyException\n\
# 0 testMyException()(./main)\n\
# 1 main(./main)\n\
# 2 __libc_start_main(/lib/x86_64-linux-gnu/libc.so.6)\n\
");
THROW_TEST(testMyException1, MyException("Hello"), MyException, "\
MyException\n\
# 0 testMyException1()(./main)\n\
# 1 main(./main)\n\
# 2 __libc_start_main(/lib/x86_64-linux-gnu/libc.so.6)\n\
");

int testStdVector() {
  try {
    std::vector<int> v;
    v.at(0);
  } catch (const std::exception &e) {
    std::cout << "std::exception(vector.at(0)) caught: " << &e << std::endl;
    //    std::cout << "stack: " << std::endl << get_stack_trace_names(e);
    std::string simpleStack=getStackTrace(e)->getSimpleStackTrace();
    std::string expected="\
std::out_of_range\n\
# 0 std::__throw_out_of_range_fmt(char const*, ...)(/usr/lib/x86_64-linux-gnu/libstdc++.so.6)\n\
# 1 std::vector<int, std::allocator<int> >::_M_range_check(unsigned long) const(./main)\n\
# 2 std::vector<int, std::allocator<int> >::at(unsigned long)(./main)\n\
# 3 testStdVector()(./main)\n\
# 4 main(./main)\n\
# 5 __libc_start_main(/lib/x86_64-linux-gnu/libc.so.6)\n\
";
    EXPECT_EQ(expected, simpleStack);
    std::cout << "stackSimple: " << std::endl << getStackTrace(e)->getSimpleStackTrace();
  } catch (...) {
    std::cout << "... caught" << std::endl;
  }
  return 0;
}

int main() {
  std::cout << "main" << std::endl;
  int error=0;
  error += testRuntimeError();
  error += testRuntimeError1();
  error += testMyException();
  error += testMyException1();
  error += testStdVector();
  std::cout << "about to return" << std::endl;
  return error;
}
