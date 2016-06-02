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
