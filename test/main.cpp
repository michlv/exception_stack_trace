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

using namespace exceptionstacktrace;

struct MyException : public std::runtime_error {
  int x;
  int y;
  MyException(const std::string &msg) : std::runtime_error(msg), x(1), y(2) {
  };
};

#define THROW_TEST(name, throw_exception, catch_exception) \
void name() { \
  try { \
    std::cout << #name << ": about to throw" << std::endl; \
    throw throw_exception; \
  } catch (const catch_exception &e) { \
    std::cout << #catch_exception << "(" << #throw_exception << ") caught: " << &e << std::endl; \
    std::cout << "stack: " << std::endl << get_stack_trace_names(e); \
    std::cout << "stackSimple: " << std::endl << getStackTrace(e)->getSimpleStackTrace(); \
  } catch (...) { \
    std::cout << "... caught" << std::endl; \
  } \
  std::cout << std::endl; \
}

THROW_TEST(testRuntimeError, std::runtime_error("Hello"), std::exception);
THROW_TEST(testRuntimeError1, std::runtime_error("Hello"), std::runtime_error);
THROW_TEST(testMyException, MyException("Hello"), std::exception);
THROW_TEST(testMyException1, MyException("Hello"), MyException);

void testStdVector() {
  try {
    std::cout << "testStdVector: about to throw" << std::endl;
    std::vector<int> v;
    v.at(0);
  } catch (const std::exception &e) {
    std::cout << "std::exception(vector.at(0)) caught: " << &e << std::endl;
    std::cout << "stack: " << std::endl << get_stack_trace_names(e);
    std::cout << "stackSimple: " << std::endl << getStackTrace(e)->getSimpleStackTrace();
  } catch (...) {
    std::cout << "... caught" << std::endl;
  }
}

int main() {
  std::cout << "main" << std::endl;
  testRuntimeError();
  testRuntimeError1();
  testMyException();
  testMyException1();
  testStdVector();
  std::cout << "about to return" << std::endl;
  return 0;
}
