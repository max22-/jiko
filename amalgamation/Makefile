BIN = jiko
CFLAGS = -std=c99 -Wall -Wextra -pedantic -MMD -MP -g
LDFLAGS = 
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
DEPS = $(OBJS:%.o=%.d)

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: run clean

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN) $(OBJS) $(DEPS) jiko.h main.c

-include $(DEPS)
