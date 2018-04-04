IDIR=src
LDIR=$(IDIR)/utils

CC=mpicc
CFLAGS=-Wall

DEPS=$(LDIR)/types.h $(LDIR)/common.h

POOL_OBJS=$(IDIR)/pool.c $(LDIR)/common.o
POOLSEQ_OBJS=$(IDIR)/poolseq.c $(LDIR)/common.o

.DEFAULT_GOAL := all
.PHONY: clean

ALL=pool poolseq
all: $(ALL)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

pool: $(POOL_OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

poolseq: $(POOLSEQ_OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(IDIR)/*.o $(IDIR)/**/*.o
	rm -f $(ALL)
