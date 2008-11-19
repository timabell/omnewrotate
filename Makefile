CC=/usr/local/openmoko/arm/bin/arm-angstrom-linux-gnueabi-gcc

.PHONY: all

all: rotate test

rotate: rotate.c
	${CC} -pthread -I /usr/local/openmoko/arm/arm-angstrom-linux-gnueabi/usr/include -L /usr/local/openmoko/arm/arm-angstrom-linux-gnueabi/usr/lib -lX11 -lXrandr rotate.c -o rotate
test: test.c
	${CC} -I /usr/local/openmoko/arm/arm-angstrom-linux-gnueabi/usr/include -L /usr/local/openmoko/arm/arm-angstrom-linux-gnueabi/usr/lib -lX11 -lXrandr test.c -o test
