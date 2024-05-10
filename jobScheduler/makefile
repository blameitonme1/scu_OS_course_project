CC = gcc
CFLAGS = -Wall -g
LDFLAGS =

COMMAND_EXECUTABLE = command
SCHEDULER_EXECUTABLE = scheduler

all: $(COMMAND_EXECUTABLE) $(SCHEDULER_EXECUTABLE)

$(COMMAND_EXECUTABLE): command.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SCHEDULER_EXECUTABLE): scheduler.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run_command: $(COMMAND_EXECUTABLE)
	./$(COMMAND_EXECUTABLE)

run_scheduler: $(SCHEDULER_EXECUTABLE)
	./$(SCHEDULER_EXECUTABLE)

clean:
	rm -f $(COMMAND_EXECUTABLE) $(SCHEDULER_EXECUTABLE) *.o core.*

.PHONY: clean run_command run_scheduler