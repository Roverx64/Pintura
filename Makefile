
FL = $(wildcard ./src/*.c)

all:
	gcc -Wno-incompatible-pointer-types -Wall -Wpedantic ${FL} -o ./pintura.o -I./Headers

clean:
	rm ./pintura.o

.PHONY: all clean
