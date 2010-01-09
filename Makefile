
CC=g++
DBG=gdb

all: compile


dbg: compile_dbg
	$(DBG) wc14.z

compile_dbg:
	$(CC) -Wall -g wc14.cpp -o wc14.z

compile:
	$(CC) -Wall wc14.cpp -o wc14.o

clean:
	rm -f a.out z *.z *.o
