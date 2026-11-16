sources := $(wildcard ./*.c)
sources_no_main := $(filter-out ./main.c, $(sources))
objects := $(sources_no_main:.c=.o)

tests := $(wildcard tests/*.c)
test_exes := $(tests:.c=)

all: run_main

run_tests: $(test_exes)
	@for test in $(test_exes); do \
		echo "Running $$test..."; \
		./$$test; \
	done

run_main: main program_new/main.elf
	./main program_new/main.elf

program/main.elf: program/main.c program/main.ld
	$(MAKE) -C program

program_new/main.elf: program_new/main.c program_new/main.ld program_new/crt0.S
	$(MAKE) -C program_new

main: main.c $(objects)
	$(CC) $(CFLAGS) $< $(objects) -o $@ -I. -Itests

$(test_exes): %: %.c $(objects)
	$(CC) $(CFLAGS) $< $(objects) -o $@ -I. -Itests

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ -I.

clean:
	rm -f $(test_exes) $(objects) main

.PHONY: all clean run_tests run_main
