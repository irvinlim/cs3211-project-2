IDIR=src
LDIR=$(IDIR)/utils

CC=mpicc
CFLAGS=-Wall

DEPS=$(LDIR)/common.h $(LDIR)/spec.h $(LDIR)/particles.h $(LDIR)/log.h
LIBS=$(LDIR)/common.o $(LDIR)/spec.o $(LDIR)/particles.o $(LDIR)/log.o

POOL_OBJS=$(IDIR)/pool.c $(LIBS)
POOLSEQ_OBJS=$(IDIR)/poolseq.c $(LIBS)

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
