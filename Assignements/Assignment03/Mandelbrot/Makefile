CC = gcc
CMPI = mpicc
CFLAGS = -std=c99 
EXE = exe
SRC = openmp.c mpi.c hybrid.c serial.c

all: $(EXE)

$(EXE): $(SRC)
	$(CC) openmp.c -o openmp.x $(CFLAGS) -fopenmp
	$(CC) serial.c -o serial.x $(CFLAGS)
	$(CMPI) mpi.c -o mpi.x $(CFLAGS)
	$(CMPI) hybrid.c -o hybrid.x $(CFLAGS) -fopenmp
clean:
	rm -f *~ *.o *.x
