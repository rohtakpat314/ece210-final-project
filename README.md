# ECE210 Final Project – Embedded LED System (ATmega328P)

## Overview

Implemented a real-time embedded application on the **ATmega328P (8-bit AVR architecture)** using an 8×8 WS2812B NeoPixel matrix and push-button inputs. The project focuses on **low-level I/O control, timing constraints, and state-driven execution** on a resource-limited microcontroller.

---

## Hardware

* **MCU:** ATmega328P (8-bit AVR, Harvard architecture)
* **Clock:** 16 MHz
* **SRAM:** 2 KB
* **Flash:** 32 KB
* **Display:** 8×8 WS2812B (serial, timing-critical protocol)
* **Input:** GPIO push-buttons (pull-down configuration)

---

## Core Implementation

### Real-Time Control Loop

* Single-threaded main loop with deterministic update timing
* Tick-based scheduling to regulate game state updates independent of render rate
* Avoided excessive blocking delays to maintain responsiveness

### Input Handling

* Direct GPIO reads via `digitalRead` abstraction
* Edge detection to prevent repeated triggers
* Minimal input design (2 buttons → directional control encoding)

### Data Representation

* Snake body stored as parallel coordinate arrays (`int x[]`, `int y[]`)
* Fixed-size buffer (bounded by SRAM constraints)
* Sequential memory access pattern (cache-friendly for AVR)

## LED Interface (WS2812B)

* Serial protocol requires **precise timing (~800 kHz)**
* Offloaded to Adafruit NeoPixel library (bit-banged signal generation)
* Custom 2D → 1D index mapping for framebuffer abstraction

---

### Instruction-Level Considerations

* Execution constrained by **8-bit ALU operations**
* Frequent use of:

  * integer arithmetic (ADD, SUB)
  * comparisons and branches (BRNE, BREQ equivalents)
* Tight loops (snake shift, collision scan) map directly to simple instruction sequences

### Timing & Determinism

* No OS / interrupts used for scheduling → fully cooperative loop
* Execution time predictable per frame
* Game speed controlled via software timing rather than hardware timers

---
