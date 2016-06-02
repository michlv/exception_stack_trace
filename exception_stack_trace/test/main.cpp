/*
    Copyright (c) 2016 Vladimir Michl

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
#include "../include/exception_stack_trace.h"

using namespace exceptionstacktrace;

struct MyException : public std::runtime_error {
  int x;
  int y;
  MyException(const std::string &msg) : std::runtime_error(msg), x(1), y(2) {
  };
};

void testRuntimeError() {
  try {
    std::cout << "about to throw" << std::endl;
    throw std::runtime_error("Hello");
  } catch (const std::exception &e) {
    std::cout << "std::exception(runtime_error) caught: " << &e << std::endl;
    std::cout << "stack: " << std::endl << get_stack_trace_names(e);
  } catch (...) {
    std::cout << "Exception caught" << std::endl;
  }
}

void testRuntimeError1() {
  try {
    std::cout << "about to throw" << std::endl;
    throw std::runtime_error("Hello");
  } catch (const std::runtime_error &e) {
    std::cout << "std::runtime_error caught: " << &e << std::endl;
    std::cout << "stack: " << std::endl << get_stack_trace_names(e);
  } catch (...) {
    std::cout << "Exception caught" << std::endl;
  }
}


void testMyException() {
  try {
    std::cout << "about to throw" << std::endl;
    throw MyException("Hello");
  } catch (const std::exception &e) {
    std::cout << "std::exception(MyException) caught: " << &e << std::endl;
    std::cout << "stack: " << std::endl << get_stack_trace_names(e);
  } catch (...) {
    std::cout << "Exception caught" << std::endl;
  }
}

void testMyException1() {
  try {
    std::cout << "about to throw" << std::endl;
    throw MyException("Hello");
  } catch (const MyException &e) {
    std::cout << "MyException caught: " << &e << std::endl;
    std::cout << "stack: " << std::endl << get_stack_trace_names(e);
  } catch (...) {
    std::cout << "Exception caught" << std::endl;
  }
}

int main() {
  std::cout << "main" << std::endl;
  testRuntimeError();
  testRuntimeError1();
  testMyException();
  testMyException1();
  std::cout << "about to return" << std::endl;
  return 0;
}
