# ======================================================================
#  Embench-IoT for Aquila SoC
# ======================================================================

EMBENCH_DIR = $(shell pwd -P)/../../..
LIBC        = elibc

RISCV_ABI = -march=rv32ima_zicsr_zifencei -mabi=ilp32
CROSS   = riscv32-unknown-elf
CCPATH  = $(RISCV)/bin

CC      = $(CCPATH)/$(CROSS)-gcc
LD      = $(CCPATH)/$(CROSS)-ld
AR      = $(CCPATH)/$(CROSS)-ar
OD      = $(CCPATH)/$(CROSS)-objdump
SIZE    = $(CCPATH)/$(CROSS)-size

# Which libgcc/newlib variant to link. Ask the compiler, but ask about plain rv32im/ilp32
# rather than $(RISCV_ABI): gcc's multilib matching is exact, so rv32ima_zicsr_zifencei
# matches no entry and falls back to the toolchain's default multilib -- which on an
# rv32imad toolchain is built for *hardware* double and so has no __adddf3/__muldf3/__ltdf2
# at all, while this target has no FPU and needs them. rv32im/ilp32 is the right subset:
# soft-float, and every instruction in it is in rv32ima too.
#
# This resolves to .../rv32im/ilp32/ on a multilib toolchain and to the base directory on a
# toolchain configured for rv32im alone, so it is correct in both layouts. Verified to
# produce byte-identical ELFs to the hand-written .../$(GCCVER)/rv32im/ilp32 path it
# replaced, on gcc 15.1.0.
LIB_ABI = -march=rv32im -mabi=ilp32
GCC_LIB = $(shell $(CC) $(LIB_ABI) -print-libgcc-file-name)
NEWLIB  = $(dir $(shell $(CC) $(LIB_ABI) -print-file-name=libc.a))

# Catch a toolchain that cannot serve this target before the link fails with a page of
# undefined soft-float symbols. Developed against gcc 15.1.0; 14.2.0 and 16.1.0 also work.
ifeq ($(wildcard $(GCC_LIB)),)
$(error no libgcc for $(LIB_ABI) under RISCV=$(RISCV) -- is $(CC) a riscv32 cross compiler?)
endif

CFLAGS  = -Wall -O3 $(RISCV_ABI) \
          -fno-builtin -mstrict-align \
          -fdata-sections -ffunction-sections \
          -I$(LIBC) -I$(EMBENCH_DIR)/support \
          -I$(EMBENCH_DIR)/examples/riscv32/Aquila \
	  -DGLOBAL_SCALE_FACTOR=1 -DWARMUP_HEAT=1 \
	  -DHAVE_BOARDSUPPORT_H

LDFLAGS = -gc-sections -Tlink.ld

BENCHMARKS = aha-mont64 crc32 depthconv edn huffbench matmult-int md5sum \
	     nettle-aes nettle-sha256 sglib-combined statemate tarfind ud \
	     nsichneu picojpeg qrduino xgboost slre wikisort

# slre and wikisort each need one symbol elibc does not provide. Take it from newlib
# *by name* rather than linking newlib globally: -lelibc is listed first, so the linker
# only searches these for symbols still unresolved, and pulls just the object that has
# one. (The notes that used to sit here, "slre: _ctype_" and "wikisort: bool", were half
# right -- slre is _ctype_, but wikisort's missing symbol is sqrt, not bool.)
#   slre     isdigit/isspace/isxdigit/tolower are macros indexing newlib's _ctype_
#            table, which lives alone in libc_a-ctype_.o and is pure data.
#   wikisort one sqrt() at libwikisort.c:386, for WikiSort's block sizing.
# $(NEWLIB) is the matching rv32im/ilp32 variant, for the same reason as libgcc above.
EXTRA_LIBS_slre     = -L$(NEWLIB) -lc
EXTRA_LIBS_wikisort = -L$(NEWLIB) -lm

# libwikisort.c:36 does `typedef uint8_t bool;` guarded by `#ifndef true`. Under C23 --
# gcc 15's default, where gcc 14 defaulted to C17 -- `bool` is a keyword while `true` is
# still not a macro, so the guard passes and the typedef is a syntax error. The source is
# pre-C23, so build that one benchmark as C17 rather than patching embench.
EXTRA_CFLAGS_wikisort = -std=gnu17


# ── Targets ──
.PHONY: all one clean $(BENCHMARKS)

all: $(BENCHMARKS)

one: $(BENCH)

$(BENCHMARKS): build/libelibc.a
	@echo ""
	@echo "======== Building $@ ========"
	@mkdir -p build/obj_$@
	@mkdir -p build/$@
	@for src in $(EMBENCH_DIR)/src/$@/*.c; do \
	    $(CC) $(CFLAGS) $(EXTRA_CFLAGS_$@) -I$(EMBENCH_DIR)/src/$@ -c $$src \
	        -o build/obj_$@/$$(basename $$src .c).o; \
	done
	$(CC) $(CFLAGS) -c $(EMBENCH_DIR)/support/main.c   -o build/obj_$@/embench_main.o
	$(CC) $(CFLAGS) -c $(EMBENCH_DIR)/support/beebsc.c -o build/obj_$@/beebsc.o
	$(CC) $(CFLAGS) -c boardsupport.c                  -o build/obj_$@/boardsupport.o
	$(LD) $(LDFLAGS) -Map=build/$@/$@.map \
	    build/obj_$@/*.o \
	    -Lbuild -lelibc $(EXTRA_LIBS_$@) $(GCC_LIB) \
	    -o build/$@/$@.elf
	$(OD) -d build/$@/$@.elf > build/$@/$@.objdump
	$(SIZE) build/$@/$@.elf
	@rm -rf build/obj_$@

build/libelibc.a:
	@echo "======== Building elibc ========"
	@mkdir -p build/elibc_tmp
	$(CC) $(CFLAGS) -c $(LIBC)/crt0.c   -o build/elibc_tmp/crt0.o
	$(CC) $(CFLAGS) -c $(LIBC)/stdio.c  -o build/elibc_tmp/stdio.o
	$(CC) $(CFLAGS) -c $(LIBC)/stdlib.c -o build/elibc_tmp/stdlib.o
	$(CC) $(CFLAGS) -c $(LIBC)/string.c -o build/elibc_tmp/string.o
	$(CC) $(CFLAGS) -c $(LIBC)/time.c   -o build/elibc_tmp/time.o
	$(CC) $(CFLAGS) -c $(LIBC)/uart.c   -o build/elibc_tmp/uart.o
	$(AR) rcs $@ build/elibc_tmp/*.o
	@rm -rf build/elibc_tmp

clean:
	rm -rf build/*
