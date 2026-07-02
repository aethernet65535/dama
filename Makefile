CC = gcc
CFLAGS = -Wall -g
TARGET = dama
OBJS = main.o core.o sysfs.o damon.o util.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c core.h sysfs.h damon.h util.h config.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
