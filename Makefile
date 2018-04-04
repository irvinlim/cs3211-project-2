IDIR=src
CC=mpic++
CFLAGS=-I $(IDIR)

DEPS=

ODIR=obj

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -Wall -c -o $@ $< $(CFLAGS)

pool: $(OBJ)
	$(CC) -Wall -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
