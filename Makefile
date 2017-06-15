TARGET = hifiveQuad
CFLAGS += -O0 -g2 -fno-builtin-printf -DUSE_PLIC -DUSE_M_TIME

BSP_BASE = ../../bsp

SRC_DIR = src
C_SRCS += $(SRC_DIR)/i2c.c
C_SRCS += $(SRC_DIR)/bno055_driver.c
C_SRCS += $(SRC_DIR)/i2c_driver.c
C_SRCS += $(BSP_BASE)/drivers/plic/plic_driver.c
C_SRCS += $(BSP_BASE)/drivers/fe300prci/fe300prci_driver.c

include $(BSP_BASE)/env/common.mk
