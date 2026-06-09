# STM32H750 Calculation of Pi

High-precision Pi computation on STM32H750VBT6 (Cortex-M7, 480MHz), using **Machin's formula** with a **decimal big-integer** engine.

## Hardware

| Item | Spec |
|------|------|
| MCU | STM32H750VBT6 (Cortex-M7, 480MHz, FPv5-DP, DSP) |
| Board | LXB750VB-P1 |
| Debugger | ST-LINK / J-LINK |
| Peripherals | LED (PC13), USART1 (115200, printf), QSPI Flash (W25Q64JV, optional) |
| HSE | 25 MHz |

## Algorithm

**Machin's formula** (1706):

```
π = 16·arctan(1/5) − 4·arctan(1/239)
```

where

```
arctan(1/x) = Σ_{k=0}^{∞} (−1)^k / ((2k+1)·x^(2k+1))
```

### Big-Integer Engine

- **Decimal base**: BASE = 10^9, each 32-bit word holds 9 decimal digits
- **Direct output**: no base-conversion needed — each word maps to 9 decimal digits
- **DSP reciprocal division**: `val / d` computed via `(val × ⌊2^64/d⌋) >> 64` with 64×64→128 multiply, avoiding hardware UDIV
- **Dynamic word-size**: early iterations only process a few words, tracking `top` for O(N²) savings
- **ITCM residency**: hot-path functions (`bn_add_top`, `bn_sub_top`, `bn_div_word_top_rec`, `dsp_div_word`) placed in ITCM for zero-wait-state execution
- **Big data in RAM_D1**: 5× ~1115-word buffers (~22KB) placed in `.bigdata` section for large-digit configurations
- **4× loop unrolling** on the divide inner loop to saturate Cortex-M7 dual-issue pipeline

## Project Structure

```
├── Core/                       # CubeMX-generated startup & HAL glue
│   ├── Inc/
│   └── Src/
│       ├── main.c              # Standalone version (self-contained)
│       ├── stm32h7xx_hal_msp.c
│       ├── stm32h7xx_it.c
│       └── system_stm32h7xx.c
├── Drivers/
│   ├── CMSIS/                  # CMSIS-Core (M7)
│   ├── STM32H7xx_HAL_Driver/   # HAL library
│   └── User/                   # User peripheral drivers (led, usart)
├── stm32h750/                  # Main application (CubeIDE GCC)
│   ├── Inc/
│   │   ├── pi.h                # Configuration macro
│   │   ├── clock.h             # System clock
│   │   ├── qspi_flash.h        # QSPI Flash driver
│   │   ├── led.h / usart.h     # Peripheral headers
│   │   └── main.h
│   ├── Src/
│   │   ├── main.c              # (alternate main, calls pi_compute)
│   │   └── pi.c                # Pi computation core
│   ├── Debug/                  # Debug build (makefile)
│   ├── Release/                # Release build (makefile)
│   └── .cproject / .project    # CubeIDE project files
├── MDK-ARM/                    # Keil MDK project
│   ├── STM32H750.uvprojx       # Keil project file
│   └── RTE/
└── LXB750VB-P1原理图.pdf        # Board schematic
```

## Configuration

All tunable parameters are in `stm32h750/Inc/pi.h`:

```c
#define PI_DIGITS_CONFIG  10000   // Digits to compute (10 ~ 100000)
#define STORE_TO_FLASH    0       // 1 = store result to QSPI Flash
#define FPU_DEMO          1       // 1 = also run Gauss-Legendre + Machin double-precision demo
```

## Build & Run

### STM32CubeIDE (GCC)

1. Open the `stm32h750/` folder as a CubeIDE workspace.
2. Select **Release** or **Debug** configuration.
3. Build and flash to target.
4. Connect USART1 (115200 baud) to view output.

### Keil MDK-ARM

1. Open `MDK-ARM/STM32H750.uvprojx`.
2. Build and download.

## Output Example

```
========================================
  STM32H750 PI 10000 Digits
  Machin formula + DSP decimal big-int
  Built with STM32CubeIDE (GCC)
========================================

>>> [FPU] Gauss-Legendre (double):
  iter 4: PI = 3.141592653589793
>>> [FPU] Machin atan:
  PI = 3.141592653589793

>>> arctan(1/5)...
>>> arctan(1/239)...
>>> Combining...

PI = 3.14159265358979323846264338327950288419716939937510...
     ... (10000 digits) ...

========================================
  Time Statistics:
  [FPU] Gauss-Legendre:  XXX ms
  arctan(1/5):           XXX ms
  arctan(1/239):         XXX ms
  Combine:               XXX ms
  Total (big-int):       XXX ms
========================================
```

## Performance Notes

- **Cortex-M7 480MHz** with zero-wait-state ITCM/DTCM
- **Decimal BASE=10^9**: output directly, no hex→dec conversion overhead
- **DSP division**: `__UMLAL`-style 64×64→128 multiply avoids `UDIV` in the hot loop (~9400 iterations saved for 10000 digits)
- **Dynamic word-size**: only active words participate in add/sub/div each iteration
- **Precomputed reciprocal** for `x^2` divisor reused across all arctan iterations
- Large buffers in `.bigdata` (RAM_D1), hot code in `.itcm` (ITCM)

## References

- Machin, J. (1706). On the computation of π.
- [Gauss-Legendre algorithm](https://en.wikipedia.org/wiki/Gauss%E2%80%93Legendre_algorithm)
- STM32H750VBT6 Reference Manual (RM0433)
- Cortex-M7 Technical Reference Manual
