IDIR=src
LDIR=$(IDIR)/utils
SDIR=$(IDIR)/simulation

CC=mpicc
CFLAGS=-Wall -Wextra -std=gnu99

DEPS=$(LDIR)/common.h $(LDIR)/multiproc.h $(LDIR)/spec.h $(LDIR)/particles.h $(LDIR)/log.h $(LDIR)/timer.h $(SDIR)/nbody.h $(SDIR)/regions.h
LIBS=$(LDIR)/common.o $(LDIR)/multiproc.o $(LDIR)/spec.o $(LDIR)/particles.o $(LDIR)/log.o $(LDIR)/timer.o $(SDIR)/nbody.o $(SDIR)/regions.o

POOL_OBJS=$(IDIR)/pool.c $(LIBS)
POOLSEQ_OBJS=$(IDIR)/poolseq.c $(LIBS)

.DEFAULT_GOAL := all
.PHONY: clean

# TEMP: Disable poolseq first, WIP
ALL=pool
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
