/**
  *************************************************************************************************
  * @file    pi.c
  * @brief   PI computation: Machin formula + decimal big-integer (BASE=10^9)
  *************************************************************************************************
  * Algorithm: Machin's formula (1706)
  *   pi = 16 * arctan(1/5) - 4 * arctan(1/239)
  *
  *   arctan(1/x) = SUM_(k=0..inf) (-1)^k / ((2k+1) * x^(2k+1))
  *   Iterative: term_0 = 1/x,  term_{k+1} = term_k / x^2
  *              result += (-1)^k * term_k / (2k+1)
  *
  * Big-integer: BASE = 10^9 (decimal), fixed-point
  *   DSP reciprocal division for 32-bit divisors
  *   Direct decimal output, no conversion needed
  *************************************************************************************************
  */

#include "pi.h"
#include "stm32h7xx_hal.h"
#include <math.h>
#include <stdio.h>
#include "arm_acle.h"

#if STORE_TO_FLASH
#include "qspi_flash.h"
#endif

/* ========================== Derived Constants ================================ */

#define PI_DIGITS     PI_DIGITS_CONFIG
#define BASE          1000000000U    /* 10^9: 9 decimal digits per word */
#define ARR_SIZE      ((PI_DIGITS + 8) / 9 + 3)
#define MAX_ITER_5    ((PI_DIGITS) * 72 / 100)
#define MAX_ITER_239  ((PI_DIGITS) * 22 / 100)

typedef uint32_t bignum[ARR_SIZE];

/* ========================== Static Big-Integer Buffers ========================
 * Placed in .bigdata section (RAM_D1, 512KB).
 * =========================================================================== */

static uint32_t _bn_A[ARR_SIZE] __attribute__((section(".bigdata")));  /* pi result / term_5 */
static uint32_t _bn_B[ARR_SIZE] __attribute__((section(".bigdata")));  /* result_5 */
static uint32_t _bn_D[ARR_SIZE] __attribute__((section(".bigdata")));  /* result_239 */
static uint32_t _bn_E[ARR_SIZE] __attribute__((section(".bigdata")));  /* tmp scratch */
static uint32_t _bn_F[ARR_SIZE] __attribute__((section(".bigdata")));  /* tmp scratch */

typedef struct {
	uint32_t lo;
	uint32_t hi;
} u64_t;

/* ========================== Forward Declarations ============================== */

static u64_t    dsp_recip(uint32_t d);
static uint32_t dsp_div_word(uint64_t val, uint32_t d, const u64_t *rec, uint32_t *rem_out);
static void     bn_zero(bignum r);
static void     bn_copy_top(bignum r, const bignum a, int top);
static void     bn_sub(bignum r, const bignum a, const bignum b);
static void     bn_mul_word(bignum r, const bignum a, uint32_t m);
static int      bn_add_top(bignum r, const bignum a, const bignum b, int a_top, int b_top);
static int      bn_sub_top(bignum r, const bignum a, const bignum b, int a_top, int b_top);
static int      bn_div_word_top(bignum r, const bignum a, uint32_t d, int top);
static int      bn_div_word_top_rec(bignum r, const bignum a, uint32_t d, int top, const u64_t *recip);
static void     bn_arctan(bignum result, uint32_t x, int max_iter);
static void     bn_print_dec(const bignum pi);
#if FPU_DEMO
static double   fpu_pi_gauss(int iters);
static double   fpu_pi_machin(void);
#endif

/* ============================================================================
 * DSP Reciprocal Division
 *   rec = floor(2^64 / d)
 *   q   = (val * rec) >> 64
 *   rem = val - q*d;  if (rem >= d) { rem -= d; q++; }
 * ============================================================================ */

static u64_t dsp_recip(uint32_t d)
{
	u64_t r;
	uint64_t rec = 0xFFFFFFFFFFFFFFFFULL / d;
	r.lo = (uint32_t)(rec);
	r.hi = (uint32_t)(rec >> 32);
	return r;
}

__attribute__((section(".itcm"), always_inline))
static inline uint32_t dsp_div_word(uint64_t val, uint32_t d,
                                    const u64_t *rec, uint32_t *rem_out)
{
	uint32_t vl = (uint32_t)val;
	uint32_t vh = (uint32_t)(val >> 32);
	uint32_t rl = rec->lo;
	uint32_t rh = rec->hi;

	/* 64x64 -> 128 multiply, upper 64 bits = floor(val * rec / 2^64) */
	uint64_t p00 = (uint64_t)vl * rl;
	uint64_t p01 = (uint64_t)vl * rh;
	uint64_t p10 = (uint64_t)vh * rl;
	uint64_t p11 = (uint64_t)vh * rh;

	uint64_t mid  = (p00 >> 32) + (uint32_t)p01 + (uint32_t)p10;
	uint64_t hi   = (p01 >> 32) + (p10 >> 32) + p11 + (mid >> 32);

	uint64_t q = hi;
	uint64_t rem64 = val - q * d;
	if (rem64 >= d) { rem64 -= d; q++; }
	if (rem_out) *rem_out = (uint32_t)rem64;
	return (uint32_t)q;
}

/* ---- Basic big-integer operations (BASE = 10^9, little-endian) ---- */

static void bn_zero(bignum r)
{
	int i;
	for (i = 0; i < ARR_SIZE; i++)
		r[i] = 0;
}

static void bn_copy_top(bignum r, const bignum a, int top)
{
	int i;
	for (i = 0; i < top; i++)
		r[i] = a[i];
	for (i = top; i < ARR_SIZE; i++)
		r[i] = 0;
}

/* r = a - b (a must be >= b), decimal borrow */
static void bn_sub(bignum r, const bignum a, const bignum b)
{
	int32_t borrow = 0;
	int i;
	for (i = 0; i < ARR_SIZE; i++)
	{
		int64_t diff = (int64_t)a[i] - (int64_t)b[i] - borrow;
		if (diff < 0)
		{
			r[i] = (uint32_t)(diff + BASE);
			borrow = 1;
		}
		else
		{
			r[i] = (uint32_t)diff;
			borrow = 0;
		}
	}
}

/* r = a * m  (m is 32-bit), decimal carry */
static void bn_mul_word(bignum r, const bignum a, uint32_t m)
{
	uint64_t carry = 0;
	int i;
	for (i = 0; i < ARR_SIZE; i++)
	{
		uint64_t prod = (uint64_t)a[i] * m + carry;
		r[i] = (uint32_t)(prod % BASE);
		carry = prod / BASE;
	}
}

/* ---- Top-aware variants (for hot path dynamic word-size) ---- */

/* r = a + b, only processes up to max(a_top, b_top) words.
 * Returns new top.  Safe when r == a (in-place). */
__attribute__((section(".itcm")))
static int bn_add_top(bignum r, const bignum a, const bignum b, int a_top, int b_top)
{
	int top = (a_top > b_top) ? a_top : b_top;
	uint32_t carry = 0;
	int i;

#ifdef __clang__
#pragma clang loop unroll(enable)
#endif
	for (i = 0; i < top; i++)
	{
		uint32_t av = (i < a_top) ? a[i] : 0;
		uint32_t bv = (i < b_top) ? b[i] : 0;
		uint64_t sum = (uint64_t)av + bv + carry;
		if (sum >= BASE)
		{
			r[i] = (uint32_t)(sum - BASE);
			carry = 1;
		}
		else
		{
			r[i] = (uint32_t)sum;
			carry = 0;
		}
	}

	if (carry && top < ARR_SIZE)
	{
		r[top] = carry;
		top++;
	}

	for (i = top; i < ARR_SIZE; i++)
		r[i] = 0;

	return top;
}

/* r = a - b, only processes up to max(a_top, b_top) words.
 * Returns new top.  Safe when r == a (in-place). */
__attribute__((section(".itcm")))
static int bn_sub_top(bignum r, const bignum a, const bignum b, int a_top, int b_top)
{
	int top = a_top;
	int32_t borrow = 0;
	int i;

#ifdef __clang__
#pragma clang loop unroll(enable)
#endif
	for (i = 0; i < top; i++)
	{
		int64_t diff = (int64_t)a[i] - (int64_t)(i < b_top ? b[i] : 0) - borrow;
		if (diff < 0)
		{
			r[i] = (uint32_t)(diff + BASE);
			borrow = 1;
		}
		else
		{
			r[i] = (uint32_t)diff;
			borrow = 0;
		}
	}

	while (top > 0 && r[top - 1] == 0)
		top--;

	for (i = top; i < ARR_SIZE; i++)
		r[i] = 0;

	return top;
}

/* r = a / d  (32-bit d), DSP reciprocal. Thin wrapper. */
static int bn_div_word_top(bignum r, const bignum a, uint32_t d, int top)
{
	u64_t recip = dsp_recip(d);
	return bn_div_word_top_rec(r, a, d, top, &recip);
}

/* Like bn_div_word_top, but uses caller-provided reciprocal (precomputed once). */
__attribute__((section(".itcm")))
static int bn_div_word_top_rec(bignum r, const bignum a, uint32_t d, int top, const u64_t *recip)
{
	uint32_t rem = 0;
	int i;

	for (i = top - 1; i >= 3; i -= 4)
	{
		uint64_t v3 = (uint64_t)rem * BASE + a[i];
		r[i] = dsp_div_word(v3, d, recip, &rem);
		uint64_t v2 = (uint64_t)rem * BASE + a[i - 1];
		r[i - 1] = dsp_div_word(v2, d, recip, &rem);
		uint64_t v1 = (uint64_t)rem * BASE + a[i - 2];
		r[i - 2] = dsp_div_word(v1, d, recip, &rem);
		uint64_t v0 = (uint64_t)rem * BASE + a[i - 3];
		r[i - 3] = dsp_div_word(v0, d, recip, &rem);
	}
	for (; i >= 0; i--)
	{
		uint64_t val = (uint64_t)rem * BASE + a[i];
		r[i] = dsp_div_word(val, d, recip, &rem);
	}

	for (i = top; i < ARR_SIZE; i++)
		r[i] = 0;

	while (top > 0 && r[top - 1] == 0)
		top--;

	return top;
}

/* ================================= arctan(1/x) ================================= */

static void bn_arctan(bignum result, uint32_t x, int max_iter)
{
	uint32_t x2 = x * x;
	u64_t recip_x2 = dsp_recip(x2);
	uint32_t k;
	int term_top, res_top;

	bn_zero(result);
	res_top = 0;

	/* term = 1/x */
	{
		uint32_t *term = _bn_A;
		bn_zero(term);
		term[ARR_SIZE - 1] = 1;
		term_top = bn_div_word_top(term, term, x, ARR_SIZE);
		bn_copy_top(_bn_E, term, term_top);
	}

	if (term_top == 0) return;

	for (k = 0; k < max_iter; k++)
	{
		int div_top;
		uint32_t denom = 2 * k + 1;

		div_top = bn_div_word_top(_bn_F, _bn_E, denom, term_top);

		if (k & 1)
			res_top = bn_sub_top(result, result, _bn_F, res_top, div_top);
		else
			res_top = bn_add_top(result, result, _bn_F, res_top, div_top);

		term_top = bn_div_word_top_rec(_bn_E, _bn_E, x2, term_top, &recip_x2);
		if (term_top == 0) break;
	}
}

/* ================================ Print Decimal ================================ */

static void bn_print_dec(const bignum pi)
{
	int i;
	int frac_words = (PI_DIGITS + 8) / 9;

	printf("\r\nPI = %u.", (unsigned int)pi[ARR_SIZE - 1]);

	for (i = 1; i <= frac_words; i++)
	{
		int idx = ARR_SIZE - 1 - i;
		int is_last = (i == frac_words);
		int digs = is_last ? (PI_DIGITS - 9 * (frac_words - 1)) : 9;

		if (digs == 9)
		{
			printf("%09u", (unsigned int)pi[idx]);
		}
		else
		{
			uint32_t pow10 = 1;
			int j;
			for (j = 0; j < 9 - digs; j++) pow10 *= 10;
			printf("%0*u", digs, (unsigned int)(pi[idx] / pow10));
		}
	}
	printf("\r\n");
}

/* ================================= FPU Demo ================================== */

#if FPU_DEMO

static double fpu_pi_gauss(int iters)
{
	double a = 1.0, b = 1.0 / sqrt(2.0);
	double t = 0.25, p = 1.0;
	int i;
	for (i = 0; i < iters; i++)
	{
		double a_next = (a + b) / 2.0;
		b = sqrt(a * b);
		t = t - p * (a - a_next) * (a - a_next);
		p = 2.0 * p;
		a = a_next;
	}
	return (a + b) * (a + b) / (4.0 * t);
}

static double fpu_pi_machin(void)
{
	return 16.0 * atan(1.0 / 5.0) - 4.0 * atan(1.0 / 239.0);
}

#endif /* FPU_DEMO */

/* ============================ Public: pi_compute ==============================
 * Runs the full PI computation pipeline:
 *   FPU demo → arctan(1/5) → arctan(1/239) → combine → print → Flash → timing
 * =========================================================================== */

void pi_compute(void)
{
#if FPU_DEMO
	double       pi_fpu;
#endif
	uint32_t     t_start, cycles_per_ms;
	uint32_t     t_atan5, t_atan239, t_combine;
#if FPU_DEMO
	uint32_t     t_fpu;
#endif
#if STORE_TO_FLASH
	uint32_t     t_flash_erase = 0, t_flash_write = 0, t_flash_total = 0;
#endif

	/* Init DWT cycle counter */
	cycles_per_ms = SystemCoreClock / 1000U;
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	DWT->CYCCNT = 0;
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

	printf("\r\n========================================\r\n");
	printf("  STM32H750 PI %u Digits\r\n", (unsigned int)PI_DIGITS_CONFIG);
	printf("  Machin formula + DSP decimal big-int\r\n");
	printf("  Built with STM32CubeIDE (GCC)\r\n");
	printf("========================================\r\n\r\n");

#if FPU_DEMO
	/* -- FPU Gauss-Legendre -- */
	printf(">>> [FPU] Gauss-Legendre (double):\r\n");
	t_start = DWT->CYCCNT;
	pi_fpu = fpu_pi_gauss(4);
	t_fpu = (DWT->CYCCNT - t_start) / cycles_per_ms;
	printf("  iter 4: PI = %.15f\r\n", pi_fpu);

	/* -- FPU Machin atan -- */
	printf(">>> [FPU] Machin atan:\r\n");
	pi_fpu = fpu_pi_machin();
	printf("  PI = %.15f\r\n\r\n", pi_fpu);
#endif
	/* -- arctan(1/5) -- */
	printf(">>> arctan(1/5)...\r\n");
	t_start = DWT->CYCCNT;
	bn_zero(_bn_B);
	bn_arctan(_bn_B, 5, MAX_ITER_5);
	t_atan5 = (DWT->CYCCNT - t_start) / cycles_per_ms;

	/* -- arctan(1/239) -- */
	printf(">>> arctan(1/239)...\r\n");
	t_start = DWT->CYCCNT;
	bn_zero(_bn_D);
	bn_arctan(_bn_D, 239, MAX_ITER_239);
	t_atan239 = (DWT->CYCCNT - t_start) / cycles_per_ms;

	/* -- pi = 16 * arctan(1/5) - 4 * arctan(1/239) -- */
	printf(">>> Combining...\r\n");
	t_start = DWT->CYCCNT;
	bn_mul_word(_bn_B, _bn_B, 16);
	bn_mul_word(_bn_D, _bn_D, 4);
	bn_sub(_bn_A, _bn_B, _bn_D);
	t_combine = (DWT->CYCCNT - t_start) / cycles_per_ms;

	/* -- Print result -- */
	bn_print_dec(_bn_A);

	/* -- Store to QSPI Flash -- */
#if STORE_TO_FLASH
	QSPI_Flash_Init();
	t_start = DWT->CYCCNT;
	QSPI_Flash_Store_PI(_bn_A, PI_DIGITS, &t_flash_erase, &t_flash_write);
	t_flash_total = (DWT->CYCCNT - t_start) / cycles_per_ms;
#endif

	/* -- Timing summary -- */
	printf("\r\n========================================\r\n");
	printf("  Time Statistics:\r\n");
#if FPU_DEMO
	printf("  [FPU] Gauss-Legendre:  %u ms\r\n", (unsigned int)t_fpu);
#endif
	printf("  arctan(1/5):           %u ms\r\n", (unsigned int)t_atan5);
	printf("  arctan(1/239):         %u ms\r\n", (unsigned int)t_atan239);
	printf("  Combine:               %u ms\r\n", (unsigned int)t_combine);
	printf("  Total (big-int):       %u ms\r\n", (unsigned int)(t_atan5 + t_atan239 + t_combine));
#if STORE_TO_FLASH
	printf("  Flash Erase:           %u ms\r\n", (unsigned int)t_flash_erase);
	printf("  Flash Write:           %u ms\r\n", (unsigned int)t_flash_write);
	printf("  Flash Total (DWT):     %u ms\r\n", (unsigned int)t_flash_total);
#endif
	printf("========================================\r\n");
}
