CC = gcc
CFLAGS = -Wall -g
TARGET = dama
OBJS = main.o core.o sysfs.o

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c core.h config.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
