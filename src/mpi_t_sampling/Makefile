CC=mpicc
CPP=mpic++
BIN_CFLAGS=-g -O0 -Wall
CFLAGS=$(BIN_CFLAGS) -fPIC
LIBPATH=-L.
INCLUDE=
LIBS=-lgyan
EXE=tool.o
TESTDIR=tests

all:
	$(CC) $(CFLAGS) -c utility.c -o utility.o
	$(CPP) $(CFLAGS) -c mpi_t_sampling.cpp -o mpi_t_sampling.o
	$(CPP) $(CFLAGS) -c mpi_t_sampling_overrides.cpp -o mpi_t_sampling_overrides.o
	$(CC) $(CFLAGS) -c mpi_hooks.c -o mpi_hooks.o
	ar rcs libgyan.a mpi_t_sampling.o mpi_t_sampling_overrides.o mpi_hooks.o
	$(CC) -shared -o libgyan.so mpi_t_sampling_overrides.o utility.o mpi_t_sampling.o  mpi_hooks.o
	
	#$(CC) $(CFLAGS) $(INCLUDE) $(TESTDIR)/osu_bw.c $(LIBPATH) -o $(TESTDIR)/osu_bw $(LIBS)
	#$(CC) $(CFLAGS) $(INCLUDE) $(TESTDIR)/osu_bcast.c $(LIBPATH) -o $(TESTDIR)/osu_bcast $(LIBS)
clean:
	rm -f *.o
	rm -f $(TESTDIR)/osu_bw $(TESTDIR)/osu_bcast
	rm -f libgyan.so libgyan.a

