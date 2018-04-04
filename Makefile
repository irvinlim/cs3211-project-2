IDIR=src
CC=mpicc
CFLAGS=-Wall

DEPS = pool.h

.DEFAULT_GOAL := all

ALL=pool poolseq
all: $(ALL)

%.o: %.cpp $(DEPS)
	$(CC) -Wall -c -o $@ $< $(CFLAGS)

%.o: %.c $(DEPS)
	$(CC) -Wall -c -o $@ $< $(CFLAGS)

pool: $(IDIR)/pool.c
	$(CC) -o $@ $^ $(CFLAGS)

poolseq: $(IDIR)/poolseq.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f *.o
	rm -f $(ALL)
