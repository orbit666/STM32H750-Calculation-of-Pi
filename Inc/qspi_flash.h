#ifndef __QSPI_FLASH_H
#define __QSPI_FLASH_H

#include "stm32h7xx_hal.h"

/* W25Q64JV: 8MB, 128 blocks of 64KB, 2048 sectors of 4KB */
#define QSPI_SECTOR_SIZE    4096
#define QSPI_PAGE_SIZE      256

void QSPI_Flash_Init(void);
int  QSPI_Flash_Erase_Sector(uint32_t addr);
int  QSPI_Flash_Write_Page(uint32_t addr, const uint8_t *data, uint32_t len);
int  QSPI_Flash_Write(uint32_t addr, const uint8_t *data, uint32_t len);
int  QSPI_Flash_Store_PI(const uint32_t *pi_bignum, int digits,
                         uint32_t *erase_ms, uint32_t *write_ms);

#endif
