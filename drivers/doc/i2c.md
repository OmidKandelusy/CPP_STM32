# STM32C031 I2C Driver

This document explains the registers used in the custom CMSIS-based I2C driver for the STM32C031C6 microcontroller. It focuses on the registers involved in configuring the GPIO pins for I2C operation, initializing the I2C peripheral, and performing basic data transfers.

---

# 1. Peripheral Clock Enable

Before the GPIO pins or I2C peripheral can be configured, their clocks must be enabled through the Reset and Clock Control (RCC) peripheral.

## GPIO Clock

### Register

`RCC->IOPENR` – I/O Peripheral Enable Register

### Relevant Bit

`GPIOBEN`

### Usage in Driver

```c
RCC->IOPENR |= RCC_IOPENR_GPIOBEN;
```

This enables the clock for GPIO port B, which contains the I2C pins PB6 (SCL) and PB7 (SDA).

---

## I2C Clock

### Register

`RCC->APBENR1` – APB Peripheral Enable Register 1

### Relevant Bit

`I2C1EN`

### Usage in Driver

```c
RCC->APBENR1 |= RCC_APBENR1_I2C1EN;
```

This enables the clock for the I2C1 peripheral.

---

# 2. GPIO Pin Mode Configuration

The I2C peripheral requires control of the physical pins. Therefore, the GPIO pins must be configured for Alternate Function mode.

## Register

`GPIOB->MODER` – GPIO Mode Register

### Usage in Driver

```c
GPIOB->MODER &= ~(3U << (SCL_PIN * 2));
GPIOB->MODER |=  (GPIO_MODE_AF << (SCL_PIN * 2));

GPIOB->MODER &= ~(3U << (SDA_PIN * 2));
GPIOB->MODER |=  (GPIO_MODE_AF << (SDA_PIN * 2));
```

The MODER register contains two bits per GPIO pin.

Setting the mode to Alternate Function allows the I2C peripheral to control the pins instead of the GPIO peripheral.

---

# 3. Alternate Function Selection

Configuring a pin as Alternate Function is not sufficient on its own. The specific peripheral must also be selected.

For STM32C031:

* PB6 → I2C1_SCL
* PB7 → I2C1_SDA
* Alternate Function Number = AF6

## Register

`GPIOB->AFR[0]` – Alternate Function Register Low

### Usage in Driver

```c
GPIOB->AFR[0] &= ~(0xFU << (SCL_PIN * 4));
GPIOB->AFR[0] |=  (I2C_AF << (SCL_PIN * 4));

GPIOB->AFR[0] &= ~(0xFU << (SDA_PIN * 4));
GPIOB->AFR[0] |=  (I2C_AF << (SDA_PIN * 4));
```

The AFR register contains four bits per pin.

`AFR[0]` controls pins 0–7, while `AFR[1]` controls pins 8–15.

The value `AF6` connects the pins to the I2C1 peripheral.

---

# 4. Open-Drain Configuration

I2C requires open-drain outputs to allow multiple devices to share the same bus safely.

## Register

`GPIOB->OTYPER` – Output Type Register

### Usage in Driver

```c
GPIOB->OTYPER |= (1U << SCL_PIN) |
                 (1U << SDA_PIN);
```

Setting a bit in OTYPER configures the corresponding pin as open-drain.

In open-drain mode:

* The pin can actively drive LOW.
* The pin cannot actively drive HIGH.
* The HIGH level is provided by pull-up resistors.

This prevents bus contention when multiple devices are connected to the same I2C bus.

---

# 5. Pull-Up Configuration

I2C communication requires pull-up resistors on both SDA and SCL.

During initial development, internal pull-ups can be enabled through the GPIO peripheral.

## Register

`GPIOB->PUPDR` – Pull-Up/Pull-Down Register

### Usage in Driver

```c
GPIOB->PUPDR &= ~(3U << (SCL_PIN * 2));
GPIOB->PUPDR |=  (1U << (SCL_PIN * 2));

GPIOB->PUPDR &= ~(3U << (SDA_PIN * 2));
GPIOB->PUPDR |=  (1U << (SDA_PIN * 2));
```

The PUPDR register uses two bits per pin.

Configuration value:

```text
00 = No pull resistor
01 = Pull-up
10 = Pull-down
11 = Reserved
```

For production hardware, external pull-up resistors are generally preferred.

---

# 6. GPIO Output Speed

Higher GPIO speeds improve signal edge transitions and help achieve reliable I2C timing.

## Register

`GPIOB->OSPEEDR` – Output Speed Register

### Usage in Driver

```c
GPIOB->OSPEEDR |= (3U << (SCL_PIN * 2));
GPIOB->OSPEEDR |= (3U << (SDA_PIN * 2));
```

This configures the pins for Very High Speed operation.

Although optional, this is commonly used for I2C communication.

---

# 7. I2C Peripheral Enable

The I2C peripheral must be disabled before modifying timing-related settings.

## Register

`I2C1->CR1` – Control Register 1

### Relevant Bit

`PE` (Peripheral Enable)

### Usage in Driver

```c
I2C1->CR1 &= ~I2C_CR1_PE;
```

This disables the I2C peripheral before configuration.

After initialization:

```c
I2C1->CR1 |= I2C_CR1_PE;
```

This enables the peripheral and allows communication to begin.

---

# 8. I2C Timing Configuration

The I2C peripheral requires timing parameters that determine the clock frequency and bus timing characteristics.

## Register

`I2C1->TIMINGR` – Timing Register

### Fields

| Field  | Purpose         |
| ------ | --------------- |
| PRESC  | Clock prescaler |
| SCLL   | SCL low period  |
| SCLH   | SCL high period |
| SDADEL | SDA delay       |
| SCLDEL | SCL setup time  |

### Usage in Driver

```c
I2C1->TIMINGR = 0x0090273D;
```

This timing value was generated using STM32Cube tools for:

* I2C clock source: 48 MHz
* Bus speed: 400 kHz (Fast Mode)

The TIMINGR register determines the actual timing of I2C transactions on the bus.

---

# 9. I2C Transfer Configuration

Every I2C transaction is configured through Control Register 2.

## Register

`I2C1->CR2` – Control Register 2

### Fields Used

| Field   | Purpose                                    |
| ------- | ------------------------------------------ |
| SADD    | Slave address                              |
| NBYTES  | Number of bytes to transfer                |
| START   | Generate START condition                   |
| STOP    | Generate STOP condition                    |
| RD_WRN  | Read/Write direction                       |
| AUTOEND | Automatically generate STOP after transfer |

### Example Usage

```c
I2C1->CR2 =
    ((uint32_t)address << 1) |
    ((uint32_t)data_len << 16) |
    I2C_CR2_START |
    I2C_CR2_AUTOEND;
```

CR2 is used to describe the entire I2C transaction before it begins.

---

# 10. I2C Status Monitoring

The driver monitors the status of an ongoing transfer through the Interrupt and Status Register.

## Register

`I2C1->ISR` – Interrupt and Status Register

### Flags Used

| Flag  | Meaning                               |
| ----- | ------------------------------------- |
| BUSY  | Bus currently in use                  |
| TXIS  | Transmit buffer ready for next byte   |
| RXNE  | Receive buffer contains received data |
| TC    | Transfer complete                     |
| NACKF | Slave did not acknowledge             |

### Example Usage

```c
while (!(I2C1->ISR & I2C_ISR_TXIS))
{
    ...
}
```

The ISR register is used extensively during read and write operations to track transaction progress and detect errors.

---

# 11. Data Registers

The actual data bytes are transferred through dedicated transmit and receive registers.

## Transmit Register

### Register

`I2C1->TXDR`

### Usage

```c
I2C1->TXDR = data[i];
```

Writing to TXDR places a byte into the transmit path.

---

## Receive Register

### Register

`I2C1->RXDR`

### Usage

```c
buffer[i] = (uint8_t)I2C1->RXDR;
```

Reading RXDR retrieves a received byte from the peripheral.

---

# Summary

The custom CMSIS I2C driver primarily relies on the following registers:

| Register       | Purpose                       |
| -------------- | ----------------------------- |
| RCC->IOPENR    | Enable GPIO clock             |
| RCC->APBENR1   | Enable I2C peripheral clock   |
| GPIOB->MODER   | Alternate Function mode       |
| GPIOB->AFR[0]  | Select AF6 for I2C            |
| GPIOB->OTYPER  | Open-drain outputs            |
| GPIOB->PUPDR   | Enable pull-ups               |
| GPIOB->OSPEEDR | Configure GPIO speed          |
| I2C1->CR1      | Enable/disable I2C peripheral |
| I2C1->TIMINGR  | Configure I2C timing          |
| I2C1->CR2      | Configure transactions        |
| I2C1->ISR      | Monitor transfer status       |
| I2C1->TXDR     | Transmit data                 |
| I2C1->RXDR     | Receive data                  |
