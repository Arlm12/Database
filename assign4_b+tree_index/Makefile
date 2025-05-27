#compiler and Flags
CC := gcc
CFLAGS := -g -Wall
LDFLAGS := -lm

#Source Files
#All required files
SRCS := \
	test_assign4_1.c \
	test_expr.c \
	dberror.c \
	expr.c \
	record_mgr.c \
	rm_serializer.c \
	storage_mgr.c \
	buffer_mgr.c \
	buffer_mgr_stat.c \
	btree_mgr.c \
	h_needed.c 

#Object file conversion of .c files to .o files
OBJS := $(SRCS:.c=.o)

#Executables
#Creates the target files
TARGETS := outresult1 outresult2

#default target
#By default everything is a target
.PHONY: all 
all: $(TARGETS)

# Specific Rules for Each Executable

# test_assign4 is built from test_assign4_1.o and other common objects
test_assign4: test_assign4_1.o btree_mgr.o record_mgr.o rm_serializer.o expr.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# test_expr is built from test_expr.o and other common objects
test_expr: test_expr.o btree_mgr.o record_mgr.o rm_serializer.o expr.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Pattern Rule for Compiling .c to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Specific Dependencies
# Creating the .o binary files for the program to execute and establishing dependencies
test_assign4_1.o: test_assign4_1.c btree_mgr.h record_mgr.h rm_serializer.c expr.h storage_mgr.h dberror.h buffer_mgr.h buffer_mgr_stat.h
test_expr.o: test_expr.c btree_mgr.h record_mgr.h rm_serializer.c expr.h storage_mgr.h dberror.h buffer_mgr.h buffer_mgr_stat.h
btree_mgr.o: btree_mgr.c btree_mgr.h
record_mgr.o: record_mgr.c record_mgr.h buffer_mgr.h storage_mgr.h
rm_serializer.o:  dberror.h record_mgr.h tables.h
expr.o: expr.c expr.h dberror.h record_mgr.h
storage_mgr.o: storage_mgr.c storage_mgr.h
dberror.o: dberror.c dberror.h
buffer_mgr.o: buffer_mgr.c buffer_mgr.h dt.h storage_mgr.h
buffer_mgr_stat.o: buffer_mgr_stat.c buffer_mgr_stat.h buffer_mgr.h

# Clean
.PHONY: clean
clean:
	del /F /Q $(TARGETS) *.o *~ *.exe *testidx

# Run Targets or Execute the Program
.PHONY: run run_expr
run: test_assign4
	./test_assign4

run_expr: test_expr
	./test_expr