# STM32C031 GPIO Driver

This document explains the GPIO registers used in the custom CMSIS-based GPIO driver for the STM32C031C6 microcontroller. It focuses on the registers involved in configuring and controlling GPIO pins as general-purpose outputs.


Before any GPIO port can be used, its peripheral clock must be enabled in the Reset and Clock Control (RCC) unit.

### Register
`RCC->IOPENR` – I/O Peripheral Enable Register

### Relevant Bit
`GPIOAEN` (enables GPIOA clock)

### Usage in Driver
```c
if (port == GPIOA)
    RCC->IOPENR |= RCC_IOPENR_GPIOAEN;
```
where `RCC_IOPENR_GPIOAEN` is the bit mask that enables the clock for the GPIOA peripheral.