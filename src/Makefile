CC=/usr/local/openmoko/arm/bin/arm-angstrom-linux-gnueabi-gcc
INC=-I /usr/local/openmoko/arm/arm-angstrom-linux-gnueabi/usr/include
LIB=-L /usr/local/openmoko/arm/arm-angstrom-linux-gnueabi/usr/lib
PKGCONFIG=`/usr/local/openmoko/arm/bin/pkg-config --cflags --libs dbus-1 xrandr`

.PHONY: all clean

all: rotate

clean:
	rm -f rotate

rotate: rotate.c
	${CC} -pthread ${INC} ${PKGCONFIG} ${LIB}  rotate.c -o rotate


