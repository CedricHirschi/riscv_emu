#!/bin/sh
# End-to-end check of the guest trap handler in program/lib/trap.c:
# EBREAK is reported and resumed, then an unmapped load exits with 128 + 5.
out=$(./main program/trap_test.elf 2>&1)
code=$?

fail() { echo "FAIL: $1"; echo "$out"; exit 1; }

[ "$code" -eq 133 ] || fail "exit code $code, want 133"
echo "$out" | grep -q '^before$' || fail "missing 'before'"
echo "$out" | grep -q '^breakpoint at 0x[0-9A-F]*$' || fail "missing breakpoint report"
echo "$out" | grep -q '^after ebreak$' || fail "did not resume after EBREAK"
echo "$out" | grep -q '^trap: cause=5 mepc=0x[0-9A-F]* mtval=0x40000000$' || fail "missing load fault report"
echo "$out" | grep -q 'not reached' && fail "continued after a fatal fault"
exit 0
