# Compiler and Flags
CC := gcc
CFLAGS := -g -Wall
LDFLAGS := -lm

# Source Files
# All the files required
SRCS := \
    test_assign3_1.c \
    test_expr.c \
    dberror.c \
    expr.c \
    record_mgr.c \
    rm_serializer.c \
    storage_mgr.c \
    buffer_mgr.c \
    buffer_mgr_stat.c

# Object Files
OBJS := $(SRCS:.c=.o)

# Executables
# this creates the target files
TARGETS := outresult1 outresult2

# Default Target
# By default everything is a target
.PHONY: all
all: $(TARGETS)

# Specific Rules for Each Executable

# outresult1 is built from test_assign3_1.o and other common objects
outresult1: test_assign3_1.o dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# outresult2 is built from test_expr.o and other common objects
outresult2: test_expr.o dberror.o expr.o record_mgr.o rm_serializer.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Pattern Rule for Compiling .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Specific Dependencies
# Creating the .o binary files for the program to execute and creating the link between the files
test_assign3_1.o: dberror.h storage_mgr.h test_helper.h buffer_mgr.h buffer_mgr_stat.h
test_expr.o: dberror.h expr.h record_mgr.h tables.h test_helper.h
record_mgr.o: record_mgr.h buffer_mgr.h storage_mgr.h
expr.o: dberror.h record_mgr.h expr.h tables.h
rm_serializer.o: dberror.h tables.h record_mgr.h
buffer_mgr_stat.o: buffer_mgr_stat.h buffer_mgr.h
buffer_mgr.o: buffer_mgr.h dt.h storage_mgr.h
storage_mgr.o: storage_mgr.h
dberror.o: dberror.h

# Clean
.PHONY: clean
clean:
	del /F /Q $(TARGETS) *.o *~ *.exe

# Run Targets or execute the program
.PHONY: run run_expr
run: outresult1
	./outresult1

run_expr: outresult2
	./outresult2
