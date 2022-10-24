CC = gcc # gcc cross compiler for x86
CFLAGS = -c # all the flags for compiling

C_SOURCES = $(wildcard 	./*.c \
						./Server/*.c \
						./Client/*.c \
						./Slave/*.c)

C_HEADERS = $(wildcard 	./*.h)

OBJ = 	${C_SOURCES:.c=.o}

server.elf: ${OBJ}
	gcc -Wall ${OBJ} -o server.elf

%.o: %.c ${C_HEADERS}
	${CC} ${CFLAGS} $< -o $@

server: server.elf
	mv server.elf ./execution_space
	./execution_space/server.elf
	make clean

clean:
	$(RM) *.o
	$(RM) ./Server/*.o
	$(RM) ./Client/*.o
	$(RM) ./Slave/*.o
	$(RM) ./execution_space/*.elf