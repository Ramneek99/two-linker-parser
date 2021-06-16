# Makefile for Writing Make Files Example

# *****************************************************
# Variables to control Makefile operation

CC = gcc
# CFLAGS = -Wall -g

# ****************************************************
# Targets needed to bring the executable up to date

#main: main.o
#	$(CC) $(CFLAGS) -o main main.o
#
## The main.o target can be written more simply
#
#main.o: main.cpp
#	$(CC) $(CFLAGS) -c main.cpp

main: main.cpp
	g++ -g main.cpp -o main