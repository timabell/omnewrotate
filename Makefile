.PHONE: all

all: rotate test

rotate: rotate.c
	arm-angstrom-linux-gnueabi-gcc -I /usr/local/openmoko/arm/arm-angstrom-linux-gnueabi/usr/include -L /usr/local/openmoko/arm/arm-angstrom-linux-gnueabi/usr/lib -lX11 -lXrandr rotate.c -o rotate
test: test.c
	arm-angstrom-linux-gnueabi-gcc -I /usr/local/openmoko/arm/arm-angstrom-linux-gnueabi/usr/include -L /usr/local/openmoko/arm/arm-angstrom-linux-gnueabi/usr/lib -lX11 -lXrandr test.c -o test
