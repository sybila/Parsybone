#This file may be used for building parsybone with GNU Make utility
#For sucessful compilation GCC v4.6 or higher is required

#If your boost libraries are not in your include path, specify the path here
BOOST_PATH = .

GCC = gcc
GPP = g++
OPT = -O3

all: parsybone

sqlite.o:
	$(GCC) -o $@ -c sqlite3/sqlite3.c
	
parsybone: sqlite.o main.cpp
	$(GPP) $(OPT) -o $@ $^ -std=c++11 -I $(BOOST_PATH) -I sqlite3/
	rm sqlite.o
	
clean:
	rm -f sqlite.o parsybone