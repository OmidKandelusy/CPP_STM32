# UART Driver

This document explains the UART (USART) registers, CMSIS macros, and configuration steps used in the custom UART driver for the STM32C031C6 microcontroller. It focuses on how UART is initialized, how GPIO alternate functions are configured, and how interrupts and data transmission work.

## 1. Overview

This driver configures USART2 on the STM32C031C6 for interrupt-driven reception and polled transmission. It operates directly on CMSIS peripheral structs — no HAL layer is involved. The sections below document every hardware register that is touched, the specific bit-field written, and the engineering rationale behind each decision.

Default pin mapping (overridable via `uart_t` struct):

- TX → PA2 (AF1, USART2_TX)
- RX → PA3 (AF1, USART2_RX)
- Baud → 9600 (PCLK-derived via BRR)

---

## 2. Clock Enablement

Before any peripheral register can be written, its bus clock must be enabled. The STM32C031C6 gates all peripheral clocks through the RCC block to save power. Writing to a gated peripheral has no effect and may lock the bus.

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| `RCC->IOPENR` | `GPIOAEN` (bit 0) | `\|= 1` | Enables the AHB clock to GPIOA. Required before any MODER/AFR write to PA2/PA3 takes effect. Without this, GPIO registers read back 0 and writes are silently discarded. |
| `RCC->APBENR1` | `USART2EN` (bit 17) | `\|= 1` | Gates the APB1 clock to USART2. USART2 sits on APB1 on this device. The clock must be present before BRR or CR1 are programmed — otherwise the baud rate divider loads into an unclocked register and the peripheral never starts. |
| `RCC->APBENR2` | `USART1EN` (bit 14) | `\|= 1` | Same rationale for USART1 if selected. USART1 is on APB2 (higher speed bus), so it uses APBENR2 rather than APBENR1. |

---

## 3. GPIO Pin Configuration

Two GPIO registers must be configured per pin: the mode register (MODER) to hand control to the alternate-function mux, and the alternate-function register (AFR) to select which peripheral appears on the pin.

### 3.1 MODER — Mode Register

Each GPIO pin uses two bits in MODER. The encoding is: `00` = Input, `01` = Output, `10` = Alternate Function, `11` = Analog. The driver clears both bits first (read-modify-write) then sets `0b10`.

```c
MODER &= ~(3U << (pin * 2));    // clear
MODER |=  (0x2U << (pin * 2)); // set AF
```

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| `GPIOx->MODER` | `MODER[2*pin+1 : 2*pin]` | `0b10` | Puts the pin in Alternate Function mode so the USART peripheral (not the CPU GPIO logic) drives/samples the line. Setting Output (`01`) instead would make TX a CPU-driven GPIO — the USART TX shift register would never appear on the wire. |

### 3.2 AFR — Alternate Function Register

AFR is split into two 32-bit words: `AFR[0]` covers pins 0–7, `AFR[1]` covers pins 8–15. Each pin occupies 4 bits (a nibble). The desired AF number (1 for USART2 on PA2/PA3) is written into that nibble.

```c
AFR[pin / 8] &= ~(0xFU << ((pin % 8) * 4));
AFR[pin / 8] |=  (af   << ((pin % 8) * 4));
```

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| `GPIOx->AFR[0/1]` | Nibble for pin | `AF1 (0x1)` | Connects PA2 to USART2_TX and PA3 to USART2_RX inside the GPIO mux. AF1 is defined in the STM32C031 datasheet alternate-function table. Using AF0 would map the pins to a different peripheral (SYS/TIM) and no UART data would appear. |

---

## 4. USART Control Registers

All USART configuration happens through CR1 (Control Register 1). The peripheral must be disabled (`UE=0`) while changing most fields — writing to a live USART can corrupt an in-progress frame.

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| `USART->CR1` | `UE` (bit 0) | Clear → `0` | Disables the USART before reconfiguring. The reference manual states that M, OVER8, and BRR must not be changed while `UE=1` because the internal clock-divider state machine would be disturbed mid-frame. |
| `USART->CR1` | `M1` (bit 28), `M0` (bit 12) | Clear both → `0` | Forces 8-bit word length. `M[1:0] = 00` selects 8 data bits. Clearing both explicitly (not just relying on reset defaults) ensures the driver works correctly even if a previous boot loaded different settings into retention RAM. |
| `USART->BRR` | `BRR[15:0]` | `PCLK / baudrate` | Sets the baud rate divisor. `BRR = fPCLK / desired_baud`. For 9600 baud with a 48 MHz PCLK this yields 5000 (`0x1388`). Integer division is sufficient because `OVER8=0` (16× oversampling), which tolerates ±2.5% clock error — well within typical crystal accuracy. |
| `USART->CR1` | `TE` (bit 3) | Set → `1` | Enables the transmitter. The TX shift register and idle-line management are activated. Without `TE`, writes to TDR in `uart_write_byte` have no effect. |
| `USART->CR1` | `RE` (bit 2) | Set → `1` | Enables the receiver. Starts sampling the RX line for start bits. Without `RE`, the RXNE flag never asserts and no interrupt fires. |
| `USART->CR1` | `RXNEIE_RXFNEIE` (bit 5) | Set → `1` | Enables the RXNE (Receive-data-register Not Empty) interrupt. When a byte arrives the USART asserts this interrupt line, allowing the CPU to retrieve data via the ISR without polling. This is the foundation of the non-blocking receive model. |
| `USART->CR1` | `UE` (bit 0) | Set → `1` | Re-enables the USART after all configuration is complete. From this point the peripheral is live and the NVIC will receive RX interrupts. |

---

## 5. NVIC — Interrupt Controller

Enabling the interrupt inside the USART peripheral (`RXNEIE`) is necessary but not sufficient. The NVIC (Nested Vectored Interrupt Controller) must also be told to route the USART2 IRQ line to the CPU.

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| NVIC ISER | `USART2_IRQn` | `NVIC_EnableIRQ()` | Unmasks the USART2 interrupt in the NVIC priority table. The CMSIS `NVIC_EnableIRQ()` call writes the correct bit in the Interrupt Set-Enable Register. If this call is omitted the RXNE flag still asserts inside the USART, but no CPU exception is taken — the callback is never invoked. |

---

## 6. Interrupt Service Routine — `USART2_IRQHandler`

The ISR reads the status register (ISR) to distinguish between a normal receive event and an overrun error, then clears faults via the interrupt clear register (ICR).

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| `USART->ISR` | `RXNE_RXFNE` (bit 5) | Read, check set | Signals that RDR holds a freshly received byte. The ISR checks this flag before reading RDR to avoid consuming stale data or triggering on unrelated interrupt sources (e.g. ORE, IDLE) that share the same IRQ vector. |
| `USART->RDR` | `RDR[7:0]` | Read & mask `0xFF` | The received data register. Reading it automatically clears RXNE. The `0xFF` mask ensures only the 8 data bits are used even if M0/M1 ever changes to a 9-bit mode, preventing sign-extension bugs when the value is widened to `uint32_t` internally. |
| `USART->ISR` | `ORE` (bit 3) | Read, check set | Overrun Error flag: set when a new byte arrives before the previous RDR was read. Left uncleared it locks the RXNE path and causes all subsequent bytes to be silently dropped. |
| `USART->ICR` | `ORECF` (bit 3) | `\|= 1` | Clears the ORE flag by writing 1 to the Overrun-Error Clear Flag bit. Unlike older USART generations there is no side-effect read required — a direct write to ICR is sufficient and faster. |

---

## 7. Transmit Path — `uart_write_byte`

Transmission is polled (blocking wait) rather than interrupt-driven because the expected use-case is low-throughput debug/command output where simplicity outweighs throughput.

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| `USART->ISR` | `TXE_TXFNF` (bit 7) | Poll until set | Transmit Data Register Empty flag — asserts when TDR has been transferred to the shift register and is ready for the next byte. Waiting on this flag before writing ensures back-to-back bytes do not overwrite a byte still being serialised. A timeout counter (~2 500 000 iterations) prevents an infinite spin if the peripheral stalls (e.g. clock loss). |
| `USART->TDR` | `TDR[7:0]` | Write byte | Writing to TDR queues the byte for serialisation. The USART shifts it out at the configured baud rate. After the write, `TXE_TXFNF` de-asserts until the shift register has accepted the value. |

---

## 8. Required Initialization Order

The sequence below must be preserved. Reordering steps causes silent failures that are difficult to debug without a logic analyser.

1. Enable RCC clock to GPIO port (`IOPENR`)
2. Enable RCC clock to USART peripheral (`APBENR1` / `APBENR2`)
3. Configure GPIO `MODER` to Alternate Function (`0b10`)
4. Configure GPIO `AFR` nibble to the correct AF number
5. Disable USART (`CR1.UE = 0`)
6. Set word length bits `M0/M1 = 00` (8-bit)
7. Write `BRR` divisor
8. Enable TX and RX (`CR1.TE | CR1.RE`)
9. Enable RXNE interrupt (`CR1.RXNEIE`)
10. Enable IRQ in NVIC
11. Store user callback
12. Enable USART (`CR1.UE = 1`)

---

## 9. Return Codes

| Function | Code | Meaning |
|---|---|---|
| `uart_init` | `0` | Success |
| `uart_init` | `-1` | Null `uart` pointer passed |
| `uart_init` | `-2` | Unsupported GPIO port (neither GPIOA nor GPIOB) |
| `uart_init` | `-3` | Unsupported USART instance |
| `uart_write_byte` | `-1` | Null `uart` pointer passed |
| `uart_write_byte` | `-5` | TX timeout — `TXE_TXFNF` never asserted within ~2.5 M iterations |

---

*Register names and bit definitions sourced from STM32C031 Reference Manual (RM0490) and CMSIS device headers.*