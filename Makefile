sources := $(wildcard ./*.c)
sources_no_main := $(filter-out ./main.c, $(sources))
objects := $(sources_no_main:.c=.o)

tests := $(wildcard tests/*.c)
test_exes := $(tests:.c=)

CFLAGS := -O3 -Wall -Wextra -Werror

all: run_main

run_tests: $(test_exes)
	@for test in $(test_exes); do \
		echo "Running $$test..."; \
		./$$test; \
	done

run_main: main program/main.elf
	./main program/main.elf

program/main.elf:
	$(MAKE) --no-print-directory -C program

.PHONY: program/main.elf

main: main.c $(objects)
	$(CC) $(CFLAGS) $< $(objects) -o $@ -I. -Itests

$(test_exes): %: %.c $(objects)
	$(CC) $(CFLAGS) $< $(objects) -o $@ -I. -Itests

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -I.

clean:
	rm -f $(test_exes) $(objects) main

.PHONY: all clean run_tests run_main
