CC = arm-none-eabi-gcc
CXX = arm-none-eabi-g++
OBJCOPY = arm-none-eabi-objcopy

# project name:
PROJ_NAME := uart_print

# project path set up
ROOT   := $(abspath $(CURDIR)/../..)
PROJ   := $(ROOT)/projects/$(PROJ_NAME)
BOARD  := $(ROOT)/board
BUILD  := $(PROJ)/build
DRIVER := $(ROOT)/drivers
SUBSYS := $(ROOT)/subsystem

CFLAGS = -mcpu=cortex-m0plus -mthumb -O2 -Wall
# adding the dependency header files to the build path
CFLAGS += -I$(ROOT)/CMSIS
CFLAGS += -I$(DRIVER)/inc

CPPFLAGS = -mcpu=cortex-m0plus -mthumb -O2 -Wall
CPPFLAGS += -I$(DRIVER)/inc
CPPFLAGS += -I$(ROOT)/CMSIS
CPPFLAGS += -I$(SUBSYS)/blink


# reguired pre-defined symbols:
CFLAGS += -DSTM32C031xx


LDFLAGS = -T $(BOARD)/STM32C031c6TX_FLASH.ld

# Object files (placed in build folder)
OBJS = $(BUILD)/main.o $(BUILD)/system_stm32c0xx.o $(BUILD)/syscalls.o $(BUILD)/startup_stm32c031c6tx.o $(BUILD)/gpio.o\
		$(BUILD)/wait.o $(BUILD)/blink.o $(BUILD)/uart.o

# start building into the targe:
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

$(BUILD)/gpio.o: $(DRIVER)/src/gpio.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/wait.o: $(DRIVER)/src/wait.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/uart.o: $(DRIVER)/src/uart.c
	$(CC) $(CFLAGS) -c $< -o $@

# complie c++ files

$(BUILD)/blink.o: $(SUBSYS)/blink/blink.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

$(BUILD)/main.o: $(PROJ)/main.c
	$(CC) $(CFLAGS)  -c $< -o $@


# Compile assembly files
$(BUILD)/startup_stm32c031c6tx.o: $(BOARD)/startup_stm32c031c6tx.s
	$(CC) $(CFLAGS) -c $< -o $@
