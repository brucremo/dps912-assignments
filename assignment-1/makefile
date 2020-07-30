CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES1=intMonitor.cpp
FILES2=netMonitor.cpp
FILES3=libraries.cpp


all: clean intMonitor netMonitor

intMonitor: $(FILES1)
	$(CC) $(CFLAGS) -o intMonitor $(FILES1) $(FILES3)

netMonitor: $(FILES2)
	$(CC) $(CFLAGS) -o netMonitor $(FILES2)

clean:
	rm -f *.o intMonitor netMonitor /tmp/assign1