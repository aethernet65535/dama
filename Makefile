CC = gcc
CFLAGS = -Wall -g
TARGET = dama
OBJS = main.o core.o sysfs.o test.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c

core.o: core.c core.h log.h sysfs.h
	$(CC) $(CFLAGS) -c core.c

sysfs.o: sysfs.c sysfs.h
	$(CC) $(CFLAGS) -c sysfs.c

test.o: test.c test.h
	$(CC) $(CFLAGS) -c test.c

clean:
	rm -f $(TARGET) $(OBJS)
