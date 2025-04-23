# Directories
SRC_DIR=.

# Compiler and flags
CC=gcc
CFLAGS=-g

# Source files
CPM_FILES=diskSimulator.c cpmfsys.c
MAIN_FILES=fsysdriver.c $(CPM_FILES)

# Object files
OBJS=$(CPM_FILES:.c=.o) fsysdriver.o

# Targets
all: cpmRun clean

cpmRun: $(OBJS)
	$(CC) $(CFLAGS) -o cpmRun $(OBJS)

diskSimulator.o: diskSimulator.c diskSimulator.h
	$(CC) $(CFLAGS) -c diskSimulator.c

cpmfsys.o: cpmfsys.c cpmfsys.h
	$(CC) $(CFLAGS) -c cpmfsys.c

fsysdriver.o: fsysdriver.c
	$(CC) $(CFLAGS) -c fsysdriver.c

clean:
	rm -f *.o
