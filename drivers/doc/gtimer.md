# General-Purpose Timer Driver (TIM3)
`gtimer.c / gtimer.h` — Custom bare-metal one-shot millisecond timer using raw CMSIS headers

## 1. Overview

This driver wraps TIM3 as a one-shot millisecond delay/timeout timer with a user-supplied callback. It is designed for a single concurrent timer (no multi-channel scheduling). The tick clock is derived from a 12 MHz PCLK and prescaled to 1 kHz, giving a 1 ms per tick resolution and a maximum timeout of 65 535 ms (~65.5 seconds).

Key design constants:

| Constant | Value | Meaning |
|---|---|---|
| `GTIMER_PCLK_HZ` | `12 000 000` | APB1 peripheral clock fed to TIM3 |
| `GTIMER_DRIVER_PSC` | `11999` (`PCLK/1000 - 1`) | Prescaler: divides 12 MHz → 1 kHz tick |
| `GTIMER_DRIVER_ARR` | `999` | Auto-reload used during `gtimer_init` only (overwritten per call in `gtimer_timeout_ms`) |
| `GTIMER_MAX_MS` | `65535` | Maximum ARR value (16-bit register ceiling) |

## 2. Clock Enablement

TIM3 sits on APB1. Its register block is inaccessible — reads return 0, writes are discarded — until the RCC gates the bus clock on.

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| `RCC->APBENR1` | `TIM3EN` (bit 1) | `\|= 1` | Enables the APB1 clock to TIM3. Must be the first operation; any PSC/ARR write before this is silently lost, leading to a timer that runs at an unexpected (reset-default) rate or not at all. |


## 3. Timer Configuration Registers

All configuration is done while the timer is stopped (`CEN=0`). The update-event mechanism (`EGR_UG`) is used to push the shadow registers into the active hardware registers synchronously rather than waiting for the next overflow.

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| `TIM3->CR1` | `OPM` (bit 3) | `\|= 1` | One-Pulse Mode: the timer stops itself (clears `CEN`) automatically after one overflow. This is the hardware-enforced one-shot mechanism — without it the timer would wrap and fire the callback repeatedly until `CEN` is cleared in software, making the driver non-deterministic if the ISR is delayed. |
| `TIM3->PSC` | `PSC[15:0]` | `11999` | Prescaler register. The counter clock = `PCLK / (PSC + 1)` = `12 000 000 / 12 000` = **1 000 Hz** (1 ms per tick). This makes `ARR` directly equal to the timeout in milliseconds, simplifying the user-facing API to a single integer. |
| `TIM3->ARR` | `ARR[15:0]` | `999` (init) / `ms - 1` (per call) | Auto-Reload Register. The counter counts from 0 up to ARR, then overflows and fires the update event. During `gtimer_init` a sentinel value of 999 is loaded; the real value (`ms - 1`) is loaded by `gtimer_timeout_ms` before each shot. `ARR = ms - 1` because the count sequence is `0 … ARR` inclusive, which is `ARR + 1` ticks = the desired number of milliseconds. |
| `TIM3->EGR` | `UG` (bit 0) | `\|= 1` | Update Generation: forces an immediate update event, which copies PSC and ARR from their preload buffers into the active (shadow) registers. Without this, a new PSC or ARR value only takes effect after the *next* natural overflow — the first shot would run with stale values and fire at the wrong time. |
| `TIM3->SR` | `UIF` (bit 0) | `&= ~1` | The `UG` write above triggers a hardware update event which sets `UIF` as a side-effect. This must be cleared immediately after `EGR_UG` every time, otherwise the NVIC will see a pending interrupt before the timer has even started counting and fire the callback spuriously. |


## 4. Interrupt Configuration

Two layers must be enabled for the ISR to execute: the timer's own interrupt-enable bit, and the NVIC routing entry.

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| `TIM3->DIER` | `UIE` (bit 0) | `\|= 1` | Update Interrupt Enable: tells TIM3 to assert its IRQ line when `UIF` is set (i.e. on overflow / update event). Without this bit the NVIC input is never driven high regardless of the NVIC mask state. |
| NVIC ISER | `TIM3_IRQn` | `NVIC_EnableIRQ()` | Routes the TIM3 interrupt line to the CPU exception table. If omitted, `UIF` asserts correctly inside TIM3 but the core never vectors to `TIM3_IRQHandler` — the callback is never called and `timer_busy` is never cleared. |


## 5. Interrupt Service Routine — `TIM3_IRQHandler`

The ISR must confirm the source of the interrupt before acting, because other update causes (e.g. a trigger event) also share the same vector on some configurations.

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| `TIM3->SR` | `UIF` (bit 0) | Read, check set | Guards the callback dispatch. Confirms the interrupt was caused by a counter overflow (update event) and not by a spurious NVIC pending bit or a different TIM3 flag. |
| `TIM3->SR` | `UIF` (bit 0) | `&= ~1` | Clears the interrupt flag. Must happen *before* the callback is dispatched. If cleared after, and the callback re-arms the timer, the new `UIF` set by `EGR_UG` inside `gtimer_timeout_ms` would be wiped out, silently preventing the next interrupt from firing. |


## 6. `gtimer_stop` — Abort Sequence

Stopping the timer safely requires three operations in a specific order. Changing the order creates race conditions between the counter hardware and the NVIC pipeline.

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| `TIM3->CR1` | `CEN` (bit 0) | `&= ~1` | Halts the counter immediately. Done first so the hardware cannot overflow and set `UIF` between the next two cleanup steps. |
| `TIM3->SR` | `UIF` (bit 0) | `&= ~1` | Clears any update flag that was set before or during `CEN` being cleared. Without this a stale `UIF` would re-trigger the NVIC as soon as the pending clear below completes. |
| NVIC ICPR | `TIM3_IRQn` | `NVIC_ClearPendingIRQ()` | Removes any interrupt that already propagated into the NVIC pipeline before `CEN` was cleared. The NVIC latches IRQ assertions; clearing `UIF` in the peripheral does not retroactively un-pend an interrupt already accepted by the NVIC. If skipped, the ISR fires immediately after `gtimer_stop` returns, calling the callback for a timeout that was intentionally cancelled. |


## 7. `gtimer_timeout_ms` — Per-Shot Register Sequence

Each call reloads the timer with a fresh period and starts the one-shot count. The sequence mirrors the init order for the same reasons.

| Register | Field / Bits | Value Set | Purpose & Rationale |
|---|---|---|---|
| `TIM3->ARR` | `ARR[15:0]` | `ms - 1` | Loads the desired timeout. `ARR = ms - 1` because the counter increments from 0 through ARR inclusive before overflowing: that is `ARR + 1` ticks = `ms` milliseconds at 1 kHz. Writing `ms` directly would produce a timeout one tick too long. |
| `TIM3->EGR` | `UG` (bit 0) | `\|= 1` | Forces the new ARR value into the active shadow register before `CEN` is set. Without this, if a previous shot used a larger ARR, the counter would count up to the old ARR value before adopting the new one — the first call after changing the period would fire at the wrong time. |
| `TIM3->SR` | `UIF` (bit 0) | `&= ~1` | Clears the `UIF` side-effect from `EGR_UG` (same rationale as in `gtimer_init` — prevents a spurious immediate interrupt). |
| `TIM3->CR1` | `CEN` (bit 0) | `\|= 1` | Starts the counter. Must be the last step, after ARR and the shadow registers are fully loaded and `UIF` is clean. |


## 8. `timer_busy` Flag

`timer_busy` is a software guard (not a hardware register) that prevents a second caller from overwriting an in-progress timeout. It is set to `1` before `CEN` is enabled and cleared to `0` inside the ISR after the callback returns. `gtimer_stop` also clears it.

```c
if (timer_busy != 0) return GTIMER_TIMER_BUSY;
```

Because `timer_busy` is written from both thread context (`gtimer_timeout_ms`) and interrupt context (ISR), it is declared `volatile` to prevent the compiler from caching its value in a register across the check.


## 9. Required Initialization Order

| Step | Action |
|---|---|
| 1 | Enable `RCC->APBENR1.TIM3EN` |
| 2 | Set `TIM3->CR1.OPM` (one-shot mode) |
| 3 | Write `TIM3->PSC` (prescaler) |
| 4 | Write `TIM3->ARR` (auto-reload) |
| 5 | Write `TIM3->EGR.UG` (force shadow register update) |
| 6 | Clear `TIM3->SR.UIF` (discard EGR_UG side-effect) |
| 7 | Set `TIM3->DIER.UIE` (enable update interrupt in peripheral) |
| 8 | `NVIC_EnableIRQ(TIM3_IRQn)` (enable IRQ in NVIC) |
| 9 | Store user callback |


## 10. Return Codes

| Function | Code | Meaning |
|---|---|---|
| `gtimer_timeout_ms` | `0` | Success — timer started |
| `gtimer_timeout_ms` | `GTIMER_INVALID_TIMEOUT_INPUT` | `ms == 0` or `ms > 65535` |
| `gtimer_timeout_ms` | `GTIMER_TIMER_BUSY` | A previous timeout is still running |


*Register names and bit definitions sourced from STM32C031 Reference Manual (RM0490) and CMSIS device headers.*