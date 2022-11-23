CC = gcc # gcc cross compiler for x86
CFLAGS = -c # all the flags for compiling

C_SOURCES = $(wildcard 	./*.c ./Client/*.c)

C_HEADERS = $(wildcard 	./*.h ./Client/*.h)

OBJ = 	${C_SOURCES:.c=.o}

slave.elf: ${OBJ}
	gcc -Wall ${OBJ} -o slave.elf

%.o: %.c ${C_HEADERS}
	${CC} ${CFLAGS} $< -o $@

debug: slave.elf
	./slave.elf
	make clean

clean:
	$(RM) *.o
	$(RM) ./Client/*.o
	$(RM) *.elf