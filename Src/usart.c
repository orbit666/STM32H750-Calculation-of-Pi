#include "stm32h7xx_hal.h"
#include "usart.h"

void USART1_Init(void)
{
	/* Clocks */
	GPIO_USART1_TX_CLK_ENABLE;
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	(void)RCC->APB2ENR;

	/* PA9: AF7, TX, very-high speed */
	GPIOA->MODER   = (GPIOA->MODER & ~(3 << 18)) | (2 << 18);
	GPIOA->AFR[1]  = (GPIOA->AFR[1] & ~(0xF << 4)) | (7 << 4);
	GPIOA->OSPEEDR |= (3 << 18);

	/* PA10: AF7, RX, pull-up */
	GPIOA->MODER   = (GPIOA->MODER & ~(3 << 20)) | (2 << 20);
	GPIOA->AFR[1]  = (GPIOA->AFR[1] & ~(0xF << 8)) | (7 << 8);
	GPIOA->PUPDR   = (GPIOA->PUPDR & ~(3 << 20)) | (1 << 20);

	/* BRR = 120MHz / 115200 = 1041 + 11/16 */
	USART1->CR1 = 0;
	USART1->BRR = 0x411 | (11 << 0);
	USART1->CR2 = 0;
	USART1->CR3 = 0;
	USART1->PRESC = 0;
	USART1->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE;
}

int __io_putchar(int ch)
{
	while (!(USART1->ISR & USART_ISR_TXE_TXFNF)) {}
	USART1->TDR = (uint8_t)ch;
	return ch;
}
