CC = gcc
CFLAGS = -Wall -g

totalfiles = storage_mgr.c test_assign1_1.c dberror.c
tfiles = $(totalfiles:.c=.o)
all: start outresult

start:
	@echo "***** The program is about to start executing ******"

run:
	@echo "Running the files"
	./outresult

outresult: $(tfiles)
	$(CC) $(CFLAGS) $(tfiles) -o outresult
	@echo "The result file has been curated"
	@echo "*** The execution is completed ***"

%.o:%.c
	$(CC) $(CFLAGS) -c $<
	@echo "****** Have run and compiled the files ******"

clean:
	del *.o outresult outresult.exe
	@echo "****** All the generated files have been removed/deleted ******"