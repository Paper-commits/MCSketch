GCC = g++
CFLAGS = -std=gnu++11 -W -g
SEEFLAGS = -march=native -msse2
FILES = main.out
#SRC =   ../common/BOBHash32.cpp ../LDSketch/ldsketch.c main.cpp
SRC = main.cpp
all:$(FILES)

main.out:$(SRC)
	$(GCC) $(CFLAGS) $(SEEFLAGS) -g -o main.out $(SRC)

debug:

clean:
	rm $(all) -f *~ *.o *.out *.txt core
