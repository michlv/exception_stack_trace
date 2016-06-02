all:
	make -C src all
	make -C test all

clean:
	make -C src clean
	make -C test clean

test:
	make -C test test

.PHONY: all clean test


