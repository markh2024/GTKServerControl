CC = gcc
CFLAGS = `pkg-config --cflags gtk+-3.0` -rdynamic -g
LIBS = `pkg-config --libs gtk+-3.0`

SRCS = ServerGui.c
OBJS = $(SRCS:.c=.o)
EXEC = Servercontol

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(EXEC) $(OBJS)
