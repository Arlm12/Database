CC = gcc
CFLAGS = -g -Wall

hfiles = buffer_mgr.h buffer_mgr_stat.h dberror.h dt.h storage_mgr.h test_helper.h

combinedfiles1 = buffer_mgr.c storage_mgr.c dberror.c buffer_mgr_stat.c test_assign2_1.c

combinedfiles2 = buffer_mgr.c storage_mgr.c dberror.c buffer_mgr_stat.c test_assign2_2.c

cfiles1 = $(combinedfiles1:.c=.o)
cfiles2 = $(combinedfiles2:.c=.o)

all: start outresult1 outresult2

start: 
	@echo "**** The program is about to start executing ****"

runtest1:
	@echo "**** Running test_assign2_1 ****"
	./outresult1

runtest2:
	@echo "**** Running test_assign2_2 ****"
	./outresult2

outresult1: $(cfiles1)
	$(CC) $(CFLAGS) $(cfiles1) -o outresult1
	@echo "**** the program has run and executed successfully ****"

outresult2: $(cfiles2)
	$(CC) $(CFLAGS) $(cfiles2) -o outresult2
	@echo "**** the program has run and executed successfully ****"

%.o: %.c $(hfiles)
	$(CC) $(CFLAGS) -c $<
	@echo "**** compiled $< into $@ ****"

clean:
	del /F /Q *.o outresult1 outresult2 outresult1.exe outresult2.exe
	@echo "**** All the generated files have been removed ****"

.PHONY: all clean start runtest1 runtest2
