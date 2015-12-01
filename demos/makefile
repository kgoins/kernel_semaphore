CC = gcc
CFLAGS = -Wall -ansi -pthread
SRC = list_test.c
OBJS = $(SRC:.c=.o)

EXEC = run

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

$(OBJS): $(SRC)
	$(CC) $(CFLAGS) $< -c $^

clean:
	rm *.o $(EXEC)
