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

.PHONY: run clean format todo memcheck amalgamate

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN) $(OBJS) $(DEPS)
	make -C amalgamation clean

format:
	clang-format -i *.h *.c

todo:
	grep -r -n "TODO" --exclude-dir=".git" .

memcheck: $(BIN)
	valgrind --leak-check=full --show-leak-kinds=all \
			--track-origins=yes --verbose \
         	--log-file=valgrind-out.txt \
         	./$(BIN)

amalgamate:
	python amalgamation.py
	make -C amalgamation

-include $(DEPS)
