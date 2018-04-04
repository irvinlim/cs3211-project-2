IDIR=src
LDIR=$(IDIR)/utils

CC=mpicc
CFLAGS=-Wall

DEPS=$(LDIR)/common.o $(LDIR)/particles.o

POOL_OBJS=$(IDIR)/pool.c $(DEPS)
POOLSEQ_OBJS=$(IDIR)/poolseq.c $(DEPS)

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
