
FL = $(wildcard ./src/*.c)

all:
	gcc -Wno-incompatible-pointer-types -Wpedantic ${FL} -o ./pintura.o -I./Headers

clean:
	rm ./pintura.o

.PHONY: all clean
