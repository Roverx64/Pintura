
FL = $(wildcard ./src/*.c)

all:
	gcc -Wno-incompatible-pointer-types -Wall ${FL} -o ./pintura.o -I./Headers

clean:
	rm ./pintura.o

.PHONY: all clean
