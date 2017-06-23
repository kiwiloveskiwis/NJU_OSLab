UTIL_DIR       := util
BIN_DIR        := bin
OBJ_DIR        := obj
LIB_DIR        := lib
BOOT_DIR       := boot
KERNEL_DIR     := kernel
USER_DIR       := game

CFLAGS_SHORT := -Wall -Werror -Wfatal-errors
CFLAGS_SHORT += -Wno-unknown-pragmas -Wno-error=unknown-pragmas
CFLAGS_SHORT += -std=gnu11 -m32
CFLAGS_SHORT += -I .

BOOT   := $(BIN_DIR)/boot.bin
KERNEL := $(BIN_DIR)/kernel.bin
USER   := $(BIN_DIR)/user.bin
IMAGE  := $(BIN_DIR)/disk.bin

CC      := gcc
LD      := ld
OBJCOPY := objcopy
DD      := dd
QEMU    := qemu-system-i386
GDB     := gdb

CFLAGS := -Wall -Werror -Wfatal-errors #开启所有警告, 视警告为错误, 第一个错误结束编译
CFLAGS += -MD #生成依赖文件
CFLAGS += -std=gnu11 -m32 -c #编译标准, 目标架构, 只编译
CFLAGS += -I . #头文件搜索目录
CFLAGS += -O0 #不开优化, 方便调试
CFLAGS += -fno-builtin #禁止内置函数
CFLAGS += -ggdb3 #GDB调试信息
CFLAGS += -fno-stack-protector # Make qemu runnable on Mint

QEMU_OPTIONS := -serial stdio #以标准输入输为串口(COM1)
# QEMU_OPTIONS += -d int #输出中断信息
QEMU_OPTIONS += -monitor telnet:127.0.0.1:1111,server,nowait #telnet monitor
QEMU_OPTIONS += -m 256M

QEMU_DEBUG_OPTIONS := -S #启动不执行
QEMU_DEBUG_OPTIONS += -s #GDB调试服务器: 127.0.0.1:1234

GDB_OPTIONS := -ex "target remote 127.0.0.1:1234"
GDB_OPTIONS += -ex "symbol $(KERNEL)"
GDB_OPTIONS += -ex "b *0xf01017d7"
GDB_OPTIONS += -ex "b *0x80488d9"


MYFS_READ      := $(BIN_DIR)/read_myfs
MYFS_WRITE     := $(BIN_DIR)/write_myfs



OBJ_LIB_DIR    := $(OBJ_DIR)/$(LIB_DIR)
OBJ_BOOT_DIR   := $(OBJ_DIR)/$(BOOT_DIR)
OBJ_KERNEL_DIR := $(OBJ_DIR)/$(KERNEL_DIR)
OBJ_USER_DIR   := $(OBJ_DIR)/$(USER_DIR)

LD_SCRIPT := $(shell find $(KERNEL_DIR) -name "*.ld")

LIB_C := $(wildcard $(LIB_DIR)/*.c)
LIB_O := $(LIB_C:%.c=$(OBJ_DIR)/%.o)


BOOT_S := $(wildcard $(BOOT_DIR)/*.S)
BOOT_C := $(wildcard $(BOOT_DIR)/*.c)
BOOT_O := $(BOOT_S:%.S=$(OBJ_DIR)/%.o)
BOOT_O += $(BOOT_C:%.c=$(OBJ_DIR)/%.o)

KERNEL_C := $(shell find $(KERNEL_DIR) -name "*.c")
KERNEL_S := $(wildcard $(KERNEL_DIR)/*.S)
KERNEL_O := $(KERNEL_C:%.c=$(OBJ_DIR)/%.o)
KERNEL_O += $(KERNEL_S:%.S=$(OBJ_DIR)/%.o)

USER_C := $(shell find $(USER_DIR) -name "*.c")
USER_O := $(USER_C:%.c=$(OBJ_DIR)/%.o)

all: $(IMAGE) $(MYFS_READ)

$(IMAGE): $(BOOT) $(KERNEL) $(USER)  $(MYFS_WRITE)
	@mkdir -p $(BIN_DIR)
	$(DD) if=/dev/zero of=$(IMAGE) count=4099       > /dev/null # 准备磁盘文件
	$(DD) if=$(BOOT) of=$(IMAGE) conv=notrunc          > /dev/null # 填充 boot loader
	@$(MYFS_WRITE) $(IMAGE) kernel.bin                  < $(KERNEL)
	@$(MYFS_WRITE) $(IMAGE) user.bin                    < $(USER)

$(BOOT): $(BOOT_O)
	@mkdir -p $(BIN_DIR)
	$(LD) -e start -Ttext=0x7C00 -m elf_i386 -nostdlib -o $@.out $^
	$(OBJCOPY) --strip-all --only-section=.text --output-target=binary $@.out $@
	@rm $@.out
	ruby mbr.rb $@

$(OBJ_BOOT_DIR)/%.o: $(BOOT_DIR)/%.S
	@mkdir -p $(OBJ_BOOT_DIR)
	$(CC) $(CFLAGS) -Os $< -o $@

$(OBJ_BOOT_DIR)/%.o: $(BOOT_DIR)/%.c
	@mkdir -p $(OBJ_BOOT_DIR)
	$(CC) $(CFLAGS) -Os $< -o $@

$(KERNEL): $(LD_SCRIPT)
$(KERNEL): $(KERNEL_O) $(LIB_O)
	@mkdir -p $(BIN_DIR)
	$(LD) -m elf_i386 -T $(LD_SCRIPT) -nostdlib -o $@ $^ $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

$(USER): $(USER_O) $(LIB_O)
	@mkdir -p $(BIN_DIR)
	$(LD) -m elf_i386 -emain -nostdlib -o $@ $^ $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)


$(OBJ_LIB_DIR)/%.o : $(LIB_DIR)/%.c
	@mkdir -p $(OBJ_LIB_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(OBJ_KERNEL_DIR)/%.o: $(KERNEL_DIR)/%.[cS]
	mkdir -p $(OBJ_DIR)/$(dir $<)
	$(CC) $(CFLAGS) $< -o $@


$(OBJ_USER_DIR)/%.o: $(USER_DIR)/%.c
	mkdir -p $(OBJ_DIR)/$(dir $<)
	$(CC) $(CFLAGS) $< -o $@

$(BIN_DIR)/%_myfs: $(UTIL_DIR)/%_myfs.c $(UTIL_DIR)/common.c $(OBJ_KERNEL_DIR)/fs.o
	mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS_SHORT) $^ -o $@

DEPS := $(shell find -name "*.d")
-include $(DEPS)

.PHONY: qemu debug gdb clean

qemu: $(IMAGE)
	$(QEMU) $(QEMU_OPTIONS) $(IMAGE)

# Faster, but not suitable for debugging
qemu-kvm: $(IMAGE)
	$(QEMU) $(QEMU_OPTIONS) --enable-kvm $(IMAGE)

debug: $(IMAGE)
	$(QEMU) $(QEMU_DEBUG_OPTIONS) $(QEMU_OPTIONS) $(IMAGE)

gdb:
	$(GDB) $(GDB_OPTIONS)

submit:
	git ls-files | tar zcf 151250104.tar.gz -T - .git

fs-test: $(IMAGE) $(MYFS_READ)
	$(MYFS_READ) $(IMAGE) kernel.bin > /tmp/test.bin && cmp -b $(KERNEL) /tmp/test.bin
	$(MYFS_READ) $(IMAGE) user.bin   > /tmp/test.bin && cmp -b $(USER)   /tmp/test.bin

clean:
	@rm -rf $(BIN_DIR) 2> /dev/null
	@rm -rf $(OBJ_DIR) 2> /dev/null
	@rm -rf $(BOOT)    2> /dev/null
	@rm -rf $(KERNEL)  2> /dev/null
	@rm -rf $(IMAGE)   2> /dev/null
