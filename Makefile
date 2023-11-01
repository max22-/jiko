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
	rm -f $(BIN) $(OBJS) $(DEPS)

format:
	clang-format -i *.h *.c

todo:
	grep -r -n "TODO" --exclude-dir=".git" .

memcheck: $(BIN)
	valgrind --leak-check=full --show-leak-kinds=all \
			--track-origins=yes --verbose \
         	--log-file=valgrind-out.txt \
         	./$(BIN)

-include $(DEPS)
