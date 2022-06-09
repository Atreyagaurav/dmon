CC = gcc
CFLAGS = -Wall `(pkg-config --libs --cflags dbus-1)`
DEPS = c_progress/c_progress.h dmon.h
OBJ = c_progress/c_progress.o dmon.o demo.o dmon-send.o dmon-monitor.o

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

all: demo dmon-send dmon-monitor

install: dmon-send dmon-monitor
	sudo cp dmon-send /usr/local/bin/
	sudo cp dmon-monitor /usr/local/bin/

uninstall:
	sudo rm /usr/local/bin/dmon-send
	sudo rm /usr/local/bin/dmon-monitor

clean:
	rm *.o demo dmon-send dmon-monitor

demo: c_progress/c_progress.o dmon.o demo.o
	gcc $(CFLAGS) -o $@ $^

dmon-send: c_progress/c_progress.o dmon.o dmon-send.o
	gcc $(CFLAGS) -o $@ $^

dmon-monitor: c_progress/c_progress.o dmon.o dmon-monitor.o
	gcc $(CFLAGS) -o $@ $^
