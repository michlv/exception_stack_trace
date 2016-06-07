all: main

LIBPATH=../src

clean:
	rm -f *.o main

main: main.cpp
#	g++ -Wall -rdynamic -g -o main main.cpp -Wl,-rpath=lib -L lib -Wl,--wrap=__cxa_allocate_exception -lC++stack -ldl
#	g++ -Wall -rdynamic -g -o main main.cpp -Wl,-rpath=lib -L lib -lC++stack -ldl
	g++ -Wall -rdynamic -g -o main main.cpp -Wl,-rpath=${LIBPATH} -L ${LIBPATH} -lexception_stack_trace -L/usr/src/gtest -lgtest -lpthread
#	g++ -Wall -rdynamic -g -o main main.cpp 

test: main run valgrind

run:
	./main

valgrind:
	valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./main

.PHONY: all clean valgrind

