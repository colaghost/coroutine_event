DKLIB=$(HOME)/local/lib/libevent.a

CFLAGS=-g -Wall -I$(HOME)/local/include

LDFLAGS=-lpthread -lrt 

OBJS=main.o task.o iomap.o coroutine_event.o

EXE=main

all:$(OBJS)
	gcc -o $(EXE) $(OBJS) $(DKLIB) $(LDFLAGS)

SOURCES=main.c task.c iomap.c coroutine_event.c

DEPS=.depend

$(DEPS):$(SOURCES)
	makedepend -f- -I./ -Y $(SOURCES) 2> /dev/null > $(DEPS)

include $(DEPS)

.PHONY:clean
clean:
	rm $(EXE) $(OBJS)
	rm $(DEPS)
