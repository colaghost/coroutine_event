DKLIB=$(HOME)/local/libevent/lib/libevent.a

CFLAGS=-g -Wall -I$(HOME)/local/include

LDFLAGS=-lpthread -lrt 

OBJS=main.o task.o

EXE=main

all:$(OBJS)
	@cd ./donkey_server && make 
	gcc -o $(EXE) $(OBJS) $(DKLIB) $(LDFLAGS)

SOURCES=main.c task.c

DEPS=.depend

$(DEPS):$(SOURCES)
	makedepend -f- -I./ -Y $(SOURCES) 2> /dev/null > $(DEPS)

include $(DEPS)

.PHONY:clean
clean:
	rm $(EXE) $(OBJS)
	rm $(DEPS)
