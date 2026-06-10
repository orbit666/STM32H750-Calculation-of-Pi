#ifndef __PI_H
#define __PI_H

/* ============================ User Configuration ==============================
 * Change PI_DIGITS_CONFIG to set how many digits to compute.
 *   10 ~ 100000  (more digits = more RAM + slower)
 *
 * STORE_TO_FLASH:  1 = write result to QSPI Flash,  0 = skip
 * FPU_DEMO:        1 = print FPU Gauss-Legendre + Machin,  0 = skip
 * =========================================================================== */
#define PI_DIGITS_CONFIG  10000
#define STORE_TO_FLASH    0
#define FPU_DEMO          1

void pi_compute(void);

#endif
