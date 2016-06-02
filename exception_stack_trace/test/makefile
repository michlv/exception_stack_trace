all: main

LIBPATH=../src

clean:
	rm -f *.o main

main: main.cpp
#	g++ -Wall -rdynamic -g -o main main.cpp -Wl,-rpath=lib -L lib -Wl,--wrap=__cxa_allocate_exception -lC++stack -ldl
#	g++ -Wall -rdynamic -g -o main main.cpp -Wl,-rpath=lib -L lib -lC++stack -ldl
	g++ -Wall -rdynamic -g -o main main.cpp -Wl,-rpath=${LIBPATH} -L ${LIBPATH} -lexception_stack_trace
#	g++ -Wall -rdynamic -g -o main main.cpp 
