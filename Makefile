IDIR=src
LDIR=$(IDIR)/utils
SDIR=$(IDIR)/simulation

CC=mpicc
CFLAGS=-lm -Wall -Wextra -Wno-unused-command-line-argument -std=gnu99

LLIBS=common env heatmap log multiproc particles regions spec timer
SLIBS=nbody

LLIBS_O = $(addsuffix .o, $(addprefix $(LDIR)/, $(LLIBS)))
SLIBS_O = $(addsuffix .o, $(addprefix $(SDIR)/, $(SLIBS)))

POOL_OBJS=$(IDIR)/pool.c $(LLIBS_O) $(SLIBS_O)
POOLSEQ_OBJS=$(IDIR)/poolseq.c $(LLIBS_O) $(SLIBS_O)

.DEFAULT_GOAL := all
.PHONY: clean

# TEMP: Disable poolseq first, WIP
ALL=pool
all: $(ALL)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

pool: $(POOL_OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

poolseq: $(POOLSEQ_OBJS)
	$(CC) -o $@ $^ $(CFLAGS)

clean:
	rm -f $(IDIR)/*.o $(IDIR)/**/*.o
	rm -f $(ALL)
