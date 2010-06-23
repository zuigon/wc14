
CC=g++
GDB=gdb

SIMINI_URL="http://code.jellycan.com/files/simpleini-4.13.zip"
FNAME=`echo $(SIMINI_URL) | sed -e 's/^.*\///g'`

all: compile


dbg: compile_dbg
	$(GDB) wc14.z

compile_dbg:
	$(CC) -Wall -g wc14.cpp -o wc14.z

compile:
	$(CC) -Wall wc14.cpp -o wc14

clean:
	rm -f z *.z *.o *.out wc14
	rm -r *.z.dSYM
	rm -r lib/*.zip*
	rm -r lib/*.tgz*
	rm -r lib/*.tar*

vendor:
	cd lib && wget $(SIMINI_URL)
	echo $(FNAME)
	cd lib && unzip -u $(FNAME)
	cd lib && rm $(FNAME)*
