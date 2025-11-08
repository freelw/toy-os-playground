# ============================================================================
# 变量定义
# ============================================================================

# 编译器和工具
CC       = gcc
LD       = ld
NASM     = nasm
AR       = ar

# 编译选项
CFLAGS   = -O0 -m32 -g -fno-stack-protector -fno-pie
CFLAGS  += -Wno-incompatible-pointer-types
CFLAGS  += -Wno-implicit-function-declaration
CFLAGS  += -Wno-int-conversion

LDFLAGS  = -m elf_i386 -T n.lds
NASMFLAGS_ELF = -felf32
NASMFLAGS_BIN = -l boot.lst

# 源文件
C_SOURCES = lan_main.c sys_call.c intel_err_handle.c sched.c \
            fork.c mm.c print_str.c keyboard.c exit.c

# 目标文件
C_OBJS    = $(C_SOURCES:.c=.o)
ASM_OBJS  = head.o
OBJS      = $(ASM_OBJS) $(C_OBJS)

# 用户程序目录
BIN_DIRS  = loop ls shell test

# 输出文件
KERNEL    = lan_os
BOOT      = boot.bin
IMG       = a.img
VFD       = a.vfd

# 目录
LIB_DIR   = common/lib
BIN_DIR   = bin
ROOT_FS   = root_fs

# ============================================================================
# 伪目标声明
# ============================================================================

.PHONY: all clean link boot liblansys bins
.PHONY: liblansys_clean bins_clean root_fs_clean
.PHONY: help

# ============================================================================
# 主目标
# ============================================================================

all: link liblansys bins
	dd if=$(BOOT) of=$(IMG) bs=512 count=1 conv=notrunc
	dd if=$(KERNEL) of=$(IMG) bs=512 count=62 skip=8 seek=1 conv=notrunc
	head -c 1474560 /dev/zero > $(VFD)
	dd if=$(IMG) of=$(VFD) bs=512 count=63 conv=notrunc
	node fs_patch_tool/patch.js

# ============================================================================
# 内核链接
# ============================================================================

link: $(OBJS) boot liblansys
	$(LD) $(LDFLAGS) -o $(KERNEL) $(OBJS)

# 汇编文件
head.o: head.s
	$(NASM) $(NASMFLAGS_ELF) -o $@ $<

# C 文件（一次性编译所有）
$(C_OBJS): $(C_SOURCES)
	$(CC) $(CFLAGS) -c $(C_SOURCES)

# Bootloader
boot: boot.s
	$(NASM) $(NASMFLAGS_BIN) -o $(BOOT) $<

# ============================================================================
# 库和用户程序
# ============================================================================

# 编译库
liblansys:
	$(MAKE) -C $(LIB_DIR)

# 编译并复制用户程序（使用循环）
bins:
	@for dir in $(BIN_DIRS); do \
		$(MAKE) -C $(BIN_DIR)/$$dir || exit 1; \
		$(MAKE) -C $(BIN_DIR)/$$dir cp || exit 1; \
	done

# ============================================================================
# 清理目标
# ============================================================================

clean: liblansys_clean bins_clean root_fs_clean
	-rm -f $(OBJS) $(KERNEL) $(BOOT) boot.lst $(IMG) $(VFD)

liblansys_clean:
	$(MAKE) -C $(LIB_DIR) clean

bins_clean:
	@for dir in $(BIN_DIRS); do \
		$(MAKE) -C $(BIN_DIR)/$$dir clean || true; \
	done

root_fs_clean:
	-rm -f $(ROOT_FS)/loop $(ROOT_FS)/ls $(ROOT_FS)/lan_sh $(ROOT_FS)/test

# ============================================================================
# 帮助信息
# ============================================================================

help:
	@echo "Available targets:"
	@echo "  all              - Build everything (default)"
	@echo "  clean            - Clean all build artifacts"
	@echo "  link             - Link kernel only"
	@echo "  boot             - Build bootloader only"
	@echo "  liblansys        - Build library only"
	@echo "  bins             - Build user programs only"
	@echo "  help             - Show this help message"

