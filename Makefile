all:
	gcc -o dmon dmon.c `(pkg-config --libs --cflags dbus-1)`

