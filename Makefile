CC = clang
CFLAGS = -g -std=gnu99
LFLAGS = 
LIBS = 
SOURCES = hostd.c dispatcher.c queue.c sigtrap.c
OBJECTS = $(subst .c,.o,$(SOURCES))
EXE = process hostd
.PHONY: clean help

.PHONY: debug
debug: CFLAGS += -O0 -g3
debug: $(EXE)

process : sigtrap.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@ 

hostd : hostd.o dispatcher.o queue.o
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@ 

%.o : %.c
	$(CC) $(CFLAGS) -c $< 

all : $(EXE)

clean:
	rm -f $(OBJECTS) $(EXE) *~

help:
	@echo "Valid targets:"
	@echo "  all:    generates all binary files"
	@echo "  debug:  generates debug binary files"
	@echo "  clean:  removes .o and .exe files"
