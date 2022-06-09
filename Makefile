all:
	gcc -o demo dmon.c dmon.h demo.c `(pkg-config --libs --cflags dbus-1)`

