CC = gcc
CFLAGS = -Wall
SRCS = server.c client.c
OBJS = $(SRCS:.c=.o)
TARGET = server client

all : $(TARGET)

server : server.o
	$(CC) $(CFLAGS) -o $@ $<

client : client.o
	$(CC) $(CFLAGS) -o $@ $<
	
%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean: 
	rm -f $(OBJS) $(TARGET) 
