
#######
#Changes these parameters to match your config
#######
TARGET = bno055_sensor

BSP_BASE = bsp
TOOL_DIR = /home/migueljiarr/freedom-e-sdk
CFLAGS += -O2 -fno-builtin-printf -DUSE_PLIC -DUSE_M_TIME -g
BOARD = freedom-e300-hifive1

#ifdef VERBOSE
#        Q =
#        E = @true 
#else
        Q = @
        E = @echo 
#endif

CFILES += $(shell find src -mindepth 1 -maxdepth 4 -name "*.c")
CXXFILES += $(shell find src -mindepth 1 -maxdepth 4 -name "*.cpp")

INFILES := $(CFILES) $(CXXFILES)

OBJFILES := $(CXXFILES:src/%.cpp=%) $(CFILES:src/%.c=%)
DEPFILES := $(CXXFILES:src/%.cpp=%) $(CFILES:src/%.c=%)
OFILES := $(OBJFILES:%=obj/%.o)

all: $(TARGET)
ifeq ($(MAKECMDGOALS),)
-include Makefile.dep
endif
ifneq ($(filter-out clean, $(MAKECMDGOALS)),)
-include Makefile.dep
endif

 #build the bsp
 #include $(BSP_BASE)/env/bsp.mk
include bsp.mk

.PHONY: clean all depend
.SUFFIXES:
obj/%.o: src/%.c
	$(E)C-compiling $<
	$(Q)if [ ! -d `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(Q)$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDES)
obj/%.o: src/%.cpp
	$(E)C++-compiling $<
	$(Q)if [ ! -d `dirname $@` ]; then mkdir -p `dirname $@`; fi
	$(Q)$(CC) -o $@ -c $< $(CFLAGS) $(INCLUDES)
Makefile.dep: $(CFILES) $(CXXFILES)
	$(E)Depend
	$(Q)for i in $(^); do $(CC) $(CFLAGS) $(INCLUDES) -MM "$${i}" -MT obj/`basename $${i%.*}`.o; done > $@


$(TARGET):  $(LIBWRAP) $(OFILES) $(BSP_OBJS) 
	$(E)Linking $@
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) $(BSP_OBJS) $(OFILES) -o obj/$@  $(LDFLAGS)


#############################################################
# This Section is for uploading a program to SPI Flash
#############################################################
OPENOCD_UPLOAD = $(BSP_BASE)/tools/openocd_upload.sh
OPENOCDCFG ?= $(BSP_BASE)/env/$(BOARD)/openocd.cfg
OPENOCDPATH = $(TOOL_DIR)/toolchain/bin/openocd
PROGRAM_ELF = obj/$(TARGET)

upload:
	$(OPENOCD_UPLOAD) $(OPENOCDPATH) $(PROGRAM_ELF) $(OPENOCDCFG)

#############################################################
# This Section is for launching the debugger
#############################################################

OPENOCDARGS += -f $(OPENOCDCFG)

GDB     = $(TOOL_DIR)/toolchain/bin/riscv32-unknown-elf-gdb
GDBCMDS += -ex "target extended-remote localhost:3333"
GDBARGS =

run_openocd:
	$(OPENOCDPATH) $(OPENOCDARGS)

run_gdb:
	$(GDB) $(PROGRAM_ELF) $(GDBARGS)

run_debug:
	$(OPENOCDPATH) $(OPENOCDARGS) &
	$(GDB) $(PROGRAM_ELF) $(GDBARGS) $(GDBCMDS)

clean:
	$(E)Removing files
	$(Q)rm -f $(BINFILE) obj/* Makefile.dep $(BSP_CLEAN_OBJS) *.a
