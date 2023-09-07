test : test.o
	g++ -o test test.o

test.o : test.cc allocator_stack.h memory_pool.h
	g++ -std=c++11 -c test.cc

.PHONY : clean
clean :
	rm test test.o

