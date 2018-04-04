IDIR=src
CC=mpicc
CFLAGS=-Wall

.DEFAULT_GOAL := all

ALL=pool poolseq
all: $(ALL)

pool: $(IDIR)/pool.c
	$(CC) -o $@ $^ $(CFLAGS)

poolseq: $(IDIR)/poolseq.c
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
