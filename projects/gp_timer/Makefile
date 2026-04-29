# compilers
CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++

# project name:
PROJ_NAME := gp_timer

# project path set up
ROOT   := $(abspath $(CURDIR)/../..)
PROJ   := $(ROOT)/projects/$(PROJ_NAME)
BOARD  := $(ROOT)/board
BUILD  := $(PROJ)/build
DRIVER := $(ROOT)/drivers

# C compiler flags
CFLAGS = -mcpu=cortex-m0plus -mthumb -O2 -Wall
CFLAGS += -I$(ROOT)/CMSIS
CFLAGS += -I$(ROOT)/CMSIS
CFLAGS += -I$(DRIVER)/inc
# reguired pre-defined symbols:
CFLAGS += -DSTM32C031xx

# C++ compiler flags
CPPFLAGS = -mcpu=cortex-m0plus -mthumb -O2 -Wall
CPPFLAGS += -I$(DRIVER)/inc
CPPFLAGS += -I$(ROOT)/CMSIS
CPPFLAGS += -I$(ROOT)/CMSIS

# linker flags
LDFLAGS = -T $(BOARD)/STM32C031c6TX_FLASH.ld


# Object files (placed in build folder)
OBJS = $(BUILD)/main.o $(BUILD)/system_stm32c0xx.o $(BUILD)/syscalls.o $(BUILD)/startup_stm32c031c6tx.o $(BUILD)/gtimer.o\
		$(BUILD)/main.o


# start building into the target:
TARGET = $(PROJ)/build/main.elf
all: clean_build $(BUILD) $(TARGET)
clean_build:
	rm -rf $(BUILD)
	mkdir $(BUILD)


# build rule:
$(TARGET): $(OBJS)
	$(CXX) $(CPPFLAGS) $^ $(LDFLAGS) -o $@


# Compile C files
$(BUILD)/system_stm32c0xx.o: $(BOARD)/system_stm32c0xx.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/syscalls.o: $(BOARD)/syscalls.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/gtimer.o: $(DRIVER)/src/gtimer.c
	$(CC) $(CFLAGS) -c $< -o $@


# complie c++ files
$(BUILD)/main.o: $(PROJ)/main.cpp
	$(CXX) $(CPPFLAGS)  -c $< -o $@


# Compile assembly files
$(BUILD)/startup_stm32c031c6tx.o: $(BOARD)/startup_stm32c031c6tx.s
	$(CC) $(CFLAGS) -c $< -o $@
