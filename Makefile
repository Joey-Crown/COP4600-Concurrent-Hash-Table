IDIR=../
ODIR=obj
CC=gcc
CFLAGS=-Wall -Werror -I$(IDIR) -pthread
_DEPS = hashdb.h rwlock.h common.h common_threads.h
DEPS = $(patsubst %,$(IDIR),$(_DEPS))

_OBJ = chash.o hashdb.o rwlock.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

chash: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o