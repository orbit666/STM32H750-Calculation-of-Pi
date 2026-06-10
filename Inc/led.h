#ifndef __LED_H
#define __LED_H

#define LED1_PIN                  GPIO_PIN_13
#define LED1_PORT                 GPIOC
#define __HAL_RCC_LED1_CLK_ENABLE __HAL_RCC_GPIOC_CLK_ENABLE()

#define LED1_ON       HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_RESET)
#define LED1_OFF      HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_SET)
#define LED1_Toggle   HAL_GPIO_TogglePin(LED1_PORT, LED1_PIN);

void LED_Init(void);

#endif //__LED_H
