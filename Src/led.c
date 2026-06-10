#include "stm32h7xx_hal.h"
#include "led.h"

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_LED1_CLK_ENABLE;

	HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_RESET);

	GPIO_InitStruct.Pin   = LED1_PIN;
	GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull  = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED1_PORT, &GPIO_InitStruct);
}
