/**
  *************************************************************************************************
  * @file    main.c
  * @brief   PI computation on STM32H750VBT6 Cortex-M7 480MHz
  *************************************************************************************************
  */

#include "main.h"
#include "led.h"
#include "usart.h"
#include "pi.h"
#include "clock.h"
#include <stdio.h>

/* ============================ User Configuration ==============================
 * SLEEP_MODE:
 *   0 = LED blink (no sleep, keep looping)
 *   1 = Sleep   (~mA, any interrupt wakes)
 *   2 = Stop    (~100 uA, only EXTI/RTC/RST wakes)
 *   3 = Standby (~2 uA, only PA0/RTC/RST wakes, SRAM lost)
 * =========================================================================== */
#define SLEEP_MODE  0

int main(void)
{
	SCB_EnableICache();
	SCB_EnableDCache();
	HAL_Init();
	SystemClock_Config();

	LED_Init();
	USART1_Init();

	pi_compute();




#if SLEEP_MODE == 0
	printf("\r\n  All done. LED blinking...\r\n");
	while (1)
	{
		HAL_Delay(750);
		LED1_Toggle;
	}
#elif SLEEP_MODE == 1
	printf("\r\n  All done. Entering sleep mode...\r\n");
	LED1_OFF;
	__WFI();
	while (1) {}
#elif SLEEP_MODE == 2
	printf("\r\n  All done. Entering stop mode...\r\n");
	LED1_OFF;
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	while (1) {}
#elif SLEEP_MODE == 3
	printf("\r\n  All done. Entering standby mode...\r\n");
	LED1_OFF;
	HAL_PWR_EnterSTANDBYMode();
	while (1) {}
#else
	#error "SLEEP_MODE must be 0, 1, 2, or 3"
#endif
}




void Error_Handler(void)
{
	__disable_irq();
	while (1) {}
}
