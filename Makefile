# Emulator build. Guest programs and tests have their own Makefiles:
#   make -C program        build guest ELFs
#   make -C tests run      build and run the test suite
# Everything is written to one build folder, shared with the sub-makes.

BUILD ?= $(CURDIR)/build
export BUILD

CC      := cc
AR      := gcc-ar # LTO objects need the plugin-aware archiver
CFLAGS  := -O3 -g -Wall -Wextra -Werror -flto -MMD -MP -Iinc -Iinc/devices
LDFLAGS := -O3 -g -flto

SRCS := $(wildcard src/*.c src/devices/*.c)
OBJS := $(SRCS:%.c=$(BUILD)/obj/%.o)
LIB  := $(BUILD)/libriscvemu.a # everything but main(), linked by the tests too
EMU  := $(BUILD)/main

all: $(EMU)

lib: $(LIB)

$(EMU): $(BUILD)/obj/main.o $(LIB)
	$(CC) $(LDFLAGS) $^ -o $@

$(LIB): $(OBJS)
	@rm -f $@
	$(AR) rcs $@ $^

$(BUILD)/obj/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

run_program: $(EMU)
	@$(MAKE) --no-print-directory -C program
	$(EMU) $(BUILD)/program/main.elf

run_tests:
	@$(MAKE) --no-print-directory -C tests run

clean:
	rm -rf $(BUILD)

-include $(OBJS:.o=.d) $(BUILD)/obj/main.d

.PHONY: all lib run_program run_tests clean
