all: libexception_stack_trace.so

clean:
	rm -f *.o *.a *.so

libexception_stack_trace.a: exception_stack_trace.cpp
	g++ -Wall -g -c $<
	ar rvs $@ exception_stack_trace.o

libexception_stack_trace.so: exception_stack_trace.cpp
	g++ -Wall -g -c $< -fPIC
	g++ -shared -o $@ exception_stack_trace.o -ldl
