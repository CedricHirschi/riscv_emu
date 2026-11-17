# Unit tests (test_*.c, linked against the emulator library) and the
# end-to-end guest trap check. `make -C tests run` builds what it needs.

BUILD ?= $(abspath ../build)
OUT   := $(BUILD)/tests
LIB   := $(BUILD)/libriscvemu.a
EMU   := $(BUILD)/main
ELFS  := $(BUILD)/program

CC      := cc
CFLAGS  := -O3 -Wall -Wextra -Werror -flto -MMD -MP -I../inc -I../inc/devices \
           -DPROGRAM_DIR='"$(ELFS)"'
LDFLAGS := -O3 -flto

TESTS := $(patsubst %.c,$(OUT)/%,$(wildcard test_*.c))

all: $(TESTS)

run: all emulator programs
	@for t in $(TESTS); do \
		echo "> Running $$(basename $$t)..."; \
		$$t || exit 1; \
		echo "> Finished $$(basename $$t)"; \
	done
	@echo "> Running test_trap_program.sh..."
	@./test_trap_program.sh $(EMU) $(ELFS)/trap_test.elf
	@echo "> Finished test_trap_program.sh"

$(OUT)/%: %.c $(LIB)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $< $(LIB) $(LDFLAGS) -o $@

# The root Makefile owns the emulator build; ask it instead of duplicating rules
$(LIB): FORCE
	@$(MAKE) --no-print-directory -C .. lib

emulator:
	@$(MAKE) --no-print-directory -C ..

programs:
	@$(MAKE) --no-print-directory -C ../program

clean:
	rm -rf $(OUT)

-include $(TESTS:=.d)

.PHONY: all run emulator programs clean FORCE
