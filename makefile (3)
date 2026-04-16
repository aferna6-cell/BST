# Makefile for MP5
# ECE 2230

CC = gcc

# compile flags
CFLAGS = -g -Wall
CPPFLAGS = -DVALIDATE

# link flags
LDLIBS = -lm

# object files
OBJS = bst.o lab5.o

# default target
lab5: $(OBJS)
	$(CC) $(CFLAGS) -o lab5 $(OBJS) $(LDLIBS)

# compile rules
bst.o: bst.c bst.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c bst.c

lab5.o: lab5.c bst.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -c lab5.c

# clean target
clean:
	rm -f $(OBJS) lab5