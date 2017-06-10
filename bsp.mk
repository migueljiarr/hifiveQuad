# See LICENSE for license details.

ifndef _SIFIVE_MK_COMMON
_SIFIVE_MK_COMMON := # defined

#.PHONY: bsp
#bsp: #$(TARGET)

include $(BSP_BASE)/libwrap/libwrap.mk

BOARD ?= freedom-e300-hifive1
ENV_DIR = $(BSP_BASE)/env
PLATFORM_DIR = $(ENV_DIR)/$(BOARD)

ASM_SRCS += $(ENV_DIR)/start.S
ASM_SRCS += $(ENV_DIR)/entry.S
C_SRCS += $(PLATFORM_DIR)/init.c
C_SRCS += $(BSP_BASE)/drivers/plic/plic_driver.c

LINKER_SCRIPT := $(PLATFORM_DIR)/link.lds

INCLUDES += -I$(BSP_BASE)/include
INCLUDES += -I$(BSP_BASE)/drivers/
INCLUDES += -I$(ENV_DIR)
INCLUDES += -I$(PLATFORM_DIR)

#TOOL_DIR = $(BSP_BASE)/../toolchain/bin

CC := $(TOOL_DIR)/toolchain/bin/riscv32-unknown-elf-gcc
AR := $(TOOL_DIR)/toolchain/bin/riscv32-unknown-elf-ar

LDFLAGS += -T $(LINKER_SCRIPT) -nostartfiles
LDFLAGS += -L$(ENV_DIR)

BSP_ASM_OBJS := $(ASM_SRCS:.S=.o)
BSP_C_OBJS := $(C_SRCS:.c=.o)

BSP_OBJS += $(BSP_ASM_OBJS) $(BSP_C_OBJS)
LINK_DEPS += $(LINKER_SCRIPT)

BSP_CLEAN_OBJS += $(LINK_OBJS) $(BSP_OBJS)

#$(TARGET): $(LINK_OBJS) $(LINK_DEPS)
#	$(CC) $(CFLAGS) $(INCLUDES) $(LINK_OBJS) -o $@ $(LDFLAGS)

$(BSP_ASM_OBJS): %.o: %.S $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(BSP_C_OBJS): %.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -include sys/cdefs.h -c -o $@ $<

endif # _SIFIVE_MK_COMMON
