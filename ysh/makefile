CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = ./ysh.c
EXEC = ysh

all: $(EXEC)

$(EXEC): $(SRC)
	@$(CC) $(CFLAGS) $^ -o $@

clean:
	@rm -f $(EXEC)

run: $(EXEC)
	@./$(EXEC)