sources := $(wildcard *.c)
objects := $(patsubst %.c,%.o,$(filter-out main.c,$(sources)))

tests := $(wildcard tests/*.c)
test_exes := $(tests:.c=)
deps := $(objects:.o=.d) $(test_exes:=.d) main.d

CFLAGS := -O3 -Wall -Wextra -Werror -flto -MMD -MP -I.

all: run_main

run_main: main program/main.elf
	./main program/main.elf

run_tests: $(test_exes)
	@for test in $(test_exes); do \
		echo "Running $$test..."; \
		./$$test || exit 1; \
	done

program/main.elf:
	$(MAKE) --no-print-directory -C program

main: main.c $(objects)
	$(CC) $(CFLAGS) $< $(objects) -o $@

$(test_exes): %: %.c $(objects)
	$(CC) $(CFLAGS) $< $(objects) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(test_exes) $(objects) $(deps) main
	make -C program clean

-include $(deps)

.PHONY: all clean run_tests run_main program/main.elf
