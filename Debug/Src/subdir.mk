################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (14.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/clock.c \
../Src/led.c \
../Src/main.c \
../Src/pi.c \
../Src/qspi_flash.c \
../Src/stm32h7xx_hal.c \
../Src/stm32h7xx_hal_cortex.c \
../Src/stm32h7xx_hal_dma.c \
../Src/stm32h7xx_hal_dma_ex.c \
../Src/stm32h7xx_hal_exti.c \
../Src/stm32h7xx_hal_flash.c \
../Src/stm32h7xx_hal_flash_ex.c \
../Src/stm32h7xx_hal_gpio.c \
../Src/stm32h7xx_hal_hsem.c \
../Src/stm32h7xx_hal_mdma.c \
../Src/stm32h7xx_hal_msp.c \
../Src/stm32h7xx_hal_pwr.c \
../Src/stm32h7xx_hal_pwr_ex.c \
../Src/stm32h7xx_hal_qspi.c \
../Src/stm32h7xx_hal_rcc.c \
../Src/stm32h7xx_hal_rcc_ex.c \
../Src/stm32h7xx_hal_uart.c \
../Src/stm32h7xx_hal_uart_ex.c \
../Src/stm32h7xx_it.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/system_stm32h7xx.c \
../Src/usart.c 

OBJS += \
./Src/clock.o \
./Src/led.o \
./Src/main.o \
./Src/pi.o \
./Src/qspi_flash.o \
./Src/stm32h7xx_hal.o \
./Src/stm32h7xx_hal_cortex.o \
./Src/stm32h7xx_hal_dma.o \
./Src/stm32h7xx_hal_dma_ex.o \
./Src/stm32h7xx_hal_exti.o \
./Src/stm32h7xx_hal_flash.o \
./Src/stm32h7xx_hal_flash_ex.o \
./Src/stm32h7xx_hal_gpio.o \
./Src/stm32h7xx_hal_hsem.o \
./Src/stm32h7xx_hal_mdma.o \
./Src/stm32h7xx_hal_msp.o \
./Src/stm32h7xx_hal_pwr.o \
./Src/stm32h7xx_hal_pwr_ex.o \
./Src/stm32h7xx_hal_qspi.o \
./Src/stm32h7xx_hal_rcc.o \
./Src/stm32h7xx_hal_rcc_ex.o \
./Src/stm32h7xx_hal_uart.o \
./Src/stm32h7xx_hal_uart_ex.o \
./Src/stm32h7xx_it.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/system_stm32h7xx.o \
./Src/usart.o 

C_DEPS += \
./Src/clock.d \
./Src/led.d \
./Src/main.d \
./Src/pi.d \
./Src/qspi_flash.d \
./Src/stm32h7xx_hal.d \
./Src/stm32h7xx_hal_cortex.d \
./Src/stm32h7xx_hal_dma.d \
./Src/stm32h7xx_hal_dma_ex.d \
./Src/stm32h7xx_hal_exti.d \
./Src/stm32h7xx_hal_flash.d \
./Src/stm32h7xx_hal_flash_ex.d \
./Src/stm32h7xx_hal_gpio.d \
./Src/stm32h7xx_hal_hsem.d \
./Src/stm32h7xx_hal_mdma.d \
./Src/stm32h7xx_hal_msp.d \
./Src/stm32h7xx_hal_pwr.d \
./Src/stm32h7xx_hal_pwr_ex.d \
./Src/stm32h7xx_hal_qspi.d \
./Src/stm32h7xx_hal_rcc.d \
./Src/stm32h7xx_hal_rcc_ex.d \
./Src/stm32h7xx_hal_uart.d \
./Src/stm32h7xx_hal_uart_ex.d \
./Src/stm32h7xx_it.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/system_stm32h7xx.d \
./Src/usart.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su Src/%.cyclo: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu99 -g3 -DDEBUG -DSTM32H750VBTx -DSTM32H750xx -DUSE_HAL_DRIVER -DSTM32 -DSTM32H7SINGLE -DSTM32H7 -c -I../Inc -I"C:/Users/Administrator/Desktop/STM32-PI-COMPUTER/stm32h750/CMSIS/Include" -I"C:/Users/Administrator/Desktop/STM32-PI-COMPUTER/stm32h750/CMSIS/Device/ST/STM32H7xx/Include" -I"C:/Users/Administrator/Desktop/STM32-PI-COMPUTER/stm32h750/Src" -I"C:/Users/Administrator/Desktop/STM32-PI-COMPUTER/stm32h750/STM32H7xx_HAL_Driver/Inc" -I"C:/Users/Administrator/Desktop/STM32-PI-COMPUTER/stm32h750/STM32H7xx_HAL_Driver/Src" -O3 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/clock.cyclo ./Src/clock.d ./Src/clock.o ./Src/clock.su ./Src/led.cyclo ./Src/led.d ./Src/led.o ./Src/led.su ./Src/main.cyclo ./Src/main.d ./Src/main.o ./Src/main.su ./Src/pi.cyclo ./Src/pi.d ./Src/pi.o ./Src/pi.su ./Src/qspi_flash.cyclo ./Src/qspi_flash.d ./Src/qspi_flash.o ./Src/qspi_flash.su ./Src/stm32h7xx_hal.cyclo ./Src/stm32h7xx_hal.d ./Src/stm32h7xx_hal.o ./Src/stm32h7xx_hal.su ./Src/stm32h7xx_hal_cortex.cyclo ./Src/stm32h7xx_hal_cortex.d ./Src/stm32h7xx_hal_cortex.o ./Src/stm32h7xx_hal_cortex.su ./Src/stm32h7xx_hal_dma.cyclo ./Src/stm32h7xx_hal_dma.d ./Src/stm32h7xx_hal_dma.o ./Src/stm32h7xx_hal_dma.su ./Src/stm32h7xx_hal_dma_ex.cyclo ./Src/stm32h7xx_hal_dma_ex.d ./Src/stm32h7xx_hal_dma_ex.o ./Src/stm32h7xx_hal_dma_ex.su ./Src/stm32h7xx_hal_exti.cyclo ./Src/stm32h7xx_hal_exti.d ./Src/stm32h7xx_hal_exti.o ./Src/stm32h7xx_hal_exti.su ./Src/stm32h7xx_hal_flash.cyclo ./Src/stm32h7xx_hal_flash.d ./Src/stm32h7xx_hal_flash.o ./Src/stm32h7xx_hal_flash.su ./Src/stm32h7xx_hal_flash_ex.cyclo ./Src/stm32h7xx_hal_flash_ex.d ./Src/stm32h7xx_hal_flash_ex.o ./Src/stm32h7xx_hal_flash_ex.su ./Src/stm32h7xx_hal_gpio.cyclo ./Src/stm32h7xx_hal_gpio.d ./Src/stm32h7xx_hal_gpio.o ./Src/stm32h7xx_hal_gpio.su ./Src/stm32h7xx_hal_hsem.cyclo ./Src/stm32h7xx_hal_hsem.d ./Src/stm32h7xx_hal_hsem.o ./Src/stm32h7xx_hal_hsem.su ./Src/stm32h7xx_hal_mdma.cyclo ./Src/stm32h7xx_hal_mdma.d ./Src/stm32h7xx_hal_mdma.o ./Src/stm32h7xx_hal_mdma.su ./Src/stm32h7xx_hal_msp.cyclo ./Src/stm32h7xx_hal_msp.d ./Src/stm32h7xx_hal_msp.o ./Src/stm32h7xx_hal_msp.su ./Src/stm32h7xx_hal_pwr.cyclo ./Src/stm32h7xx_hal_pwr.d ./Src/stm32h7xx_hal_pwr.o ./Src/stm32h7xx_hal_pwr.su ./Src/stm32h7xx_hal_pwr_ex.cyclo ./Src/stm32h7xx_hal_pwr_ex.d ./Src/stm32h7xx_hal_pwr_ex.o ./Src/stm32h7xx_hal_pwr_ex.su ./Src/stm32h7xx_hal_qspi.cyclo ./Src/stm32h7xx_hal_qspi.d ./Src/stm32h7xx_hal_qspi.o ./Src/stm32h7xx_hal_qspi.su ./Src/stm32h7xx_hal_rcc.cyclo ./Src/stm32h7xx_hal_rcc.d ./Src/stm32h7xx_hal_rcc.o ./Src/stm32h7xx_hal_rcc.su ./Src/stm32h7xx_hal_rcc_ex.cyclo ./Src/stm32h7xx_hal_rcc_ex.d ./Src/stm32h7xx_hal_rcc_ex.o ./Src/stm32h7xx_hal_rcc_ex.su ./Src/stm32h7xx_hal_uart.cyclo ./Src/stm32h7xx_hal_uart.d ./Src/stm32h7xx_hal_uart.o ./Src/stm32h7xx_hal_uart.su ./Src/stm32h7xx_hal_uart_ex.cyclo ./Src/stm32h7xx_hal_uart_ex.d ./Src/stm32h7xx_hal_uart_ex.o ./Src/stm32h7xx_hal_uart_ex.su ./Src/stm32h7xx_it.cyclo ./Src/stm32h7xx_it.d ./Src/stm32h7xx_it.o ./Src/stm32h7xx_it.su ./Src/syscalls.cyclo ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.cyclo ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/system_stm32h7xx.cyclo ./Src/system_stm32h7xx.d ./Src/system_stm32h7xx.o ./Src/system_stm32h7xx.su ./Src/usart.cyclo ./Src/usart.d ./Src/usart.o ./Src/usart.su

.PHONY: clean-Src

