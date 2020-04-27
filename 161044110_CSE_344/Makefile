CC = cc
COPS =
LINT = lint
LOPS = -x -u
LIBTHREAD = -lpthread
POSIXTHREAD = -D_POSIX_PTHREAD_SEMANTICS
RTLIB = -lrt

all:  pCp

pCp: hw6.c buffer.c
	$(CC) $(COPS) -o pCp hw6.c buffer.c -lm $(RTLIB) $(LIBTHREAD)

clean:
	rm -f *.o core pCp
