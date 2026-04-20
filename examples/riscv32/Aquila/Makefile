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

GCCVER  = $(shell $(CC) --version | grep gcc | sed 's/^.* //g')
GCC_LIB = -L$(RISCV)/lib/gcc/$(CROSS)/$(GCCVER)/rv32im/ilp32 -lgcc

CFLAGS  = -Wall -O3 $(RISCV_ABI) \
          -fno-builtin -mstrict-align \
          -fdata-sections -ffunction-sections \
          -I$(LIBC) -I$(EMBENCH_DIR)/support \
          -I$(EMBENCH_DIR)/examples/riscv32/Aquila \
	  -DGLOBAL_SCALE_FACTOR=1 -DWARMUP_HEAT=1 \
	  -DHAVE_BOARDSUPPORT_H

LDFLAGS = -gc-sections -Tlink.ld

BENCHMARKS = aha-mont64 crc32 depthconv edn huffbench matmult-int md5sum \
	     nettle-aes nettle-sha256 sglib-combined statemate tarfind ud
# error: nsichneu, picojpeg, qrduino, xgboost: mem overflow
# error: slre: _ctype_
# error: wikisort: bool


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
	    $(CC) $(CFLAGS) -I$(EMBENCH_DIR)/src/$@ -c $$src \
	        -o build/obj_$@/$$(basename $$src .c).o; \
	done
	$(CC) $(CFLAGS) -c $(EMBENCH_DIR)/support/main.c   -o build/obj_$@/embench_main.o
	$(CC) $(CFLAGS) -c $(EMBENCH_DIR)/support/beebsc.c -o build/obj_$@/beebsc.o
	$(CC) $(CFLAGS) -c boardsupport.c                  -o build/obj_$@/boardsupport.o
	$(LD) $(LDFLAGS) -Map=build/$@/$@.map \
	    build/obj_$@/*.o \
	    -Lbuild -lelibc $(GCC_LIB) \
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
