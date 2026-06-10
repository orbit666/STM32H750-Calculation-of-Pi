#include "qspi_flash.h"
#include <stdio.h>
#include <string.h>

/* ======================== W25Q64JV QSPI Flash Driver ========================
 * Based on reference example 7.QSPI-W25Q64 HAL direct R/W
 * STM32H750 QUADSPI pins:
 *   PB2  = CLK  (AF9),  PB6  = NCS  (AF10)
 *   PD11 = IO0  (AF9),  PD12 = IO1  (AF9)
 *   PE2  = IO2  (AF9),  PD13 = IO3  (AF9)
 *
 * QSPI clock: PLL2 @ 250MHz, prescaler=1 -> 125MHz
 * ============================================================================= */

#define DIG_PER_WORD    9

#define HAL_QSPI_TIMEOUT  10000U

/* ---- W25Q64 Commands ---- */
#define CMD_ENABLE_RESET    0x66
#define CMD_RESET_DEVICE    0x99
#define CMD_WRITE_ENABLE    0x06
#define CMD_READ_STATUS1    0x05
#define CMD_SECTOR_ERASE    0x20
#define CMD_PAGE_PROGRAM    0x02
#define CMD_READ_JEDEC_ID   0x9F
#define CMD_READ_DATA       0x03

static QSPI_HandleTypeDef hqspi;

/* ---- Auto-polling for BUSY bit cleared (like reference AutoPollingMemReady) ---- */
static int Flash_AutoPollingReady(uint32_t timeout)
{
    QSPI_CommandTypeDef     cmd = {0};
    QSPI_AutoPollingTypeDef cfg = {0};

    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction       = CMD_READ_STATUS1;
    cmd.AddressMode       = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_1_LINE;
    cmd.DummyCycles       = 0;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    cfg.Match           = 0;
    cfg.Mask            = 0x01;   /* BUSY bit */
    cfg.MatchMode       = QSPI_MATCH_MODE_AND;
    cfg.StatusBytesSize = 1;
    cfg.Interval        = 0x10;
    cfg.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    if (HAL_QSPI_AutoPolling(&hqspi, &cmd, &cfg, timeout) != HAL_OK)
        return -1;
    return 0;
}

/* ---- Write Enable with auto-polling WEL (like reference) ---- */
static int Flash_WriteEnable(void)
{
    QSPI_CommandTypeDef     cmd  = {0};
    QSPI_AutoPollingTypeDef cfg  = {0};

    /* Send Write Enable command */
    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction       = CMD_WRITE_ENABLE;
    cmd.AddressMode       = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_NONE;
    cmd.DummyCycles       = 0;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT) != HAL_OK)
        return -1;

    /* Auto-poll WEL bit */
    cfg.Match           = 0x02;
    cfg.Mask            = 0x02;
    cfg.MatchMode       = QSPI_MATCH_MODE_AND;
    cfg.StatusBytesSize = 1;
    cfg.Interval        = 0x10;
    cfg.AutomaticStop   = QSPI_AUTOMATIC_STOP_ENABLE;

    cmd.Instruction     = CMD_READ_STATUS1;
    cmd.DataMode        = QSPI_DATA_1_LINE;
    cmd.NbData          = 1;

    if (HAL_QSPI_AutoPolling(&hqspi, &cmd, &cfg, HAL_QSPI_TIMEOUT) != HAL_OK)
        return -1;
    return 0;
}

/* ---- Read JEDEC ID ---- */
static uint32_t Flash_ReadID(void)
{
    QSPI_CommandTypeDef cmd = {0};
    uint8_t buf[3] = {0};

    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction       = CMD_READ_JEDEC_ID;
    cmd.AddressMode       = QSPI_ADDRESS_NONE;
    cmd.AddressSize       = QSPI_ADDRESS_24_BITS;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_1_LINE;
    cmd.DummyCycles       = 0;
    cmd.NbData            = 3;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT);
    HAL_QSPI_Receive(&hqspi, buf, HAL_QSPI_TIMEOUT);

    return ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2];
}

/* ---- Reset chip (Enable Reset + Reset Device) ---- */
static void Flash_ResetChip(void)
{
    QSPI_CommandTypeDef cmd = {0};

    /* Enable Reset */
    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction       = CMD_ENABLE_RESET;
    cmd.AddressMode       = QSPI_ADDRESS_NONE;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_NONE;
    cmd.DummyCycles       = 0;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT);
    Flash_AutoPollingReady(HAL_QSPI_TIMEOUT);

    /* Reset Device */
    cmd.Instruction = CMD_RESET_DEVICE;
    HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT);
    Flash_AutoPollingReady(HAL_QSPI_TIMEOUT);
}

/* ======================== Public API ======================== */

void QSPI_Flash_Init(void)
{
    uint32_t i, id;
    uint8_t *p;
    GPIO_InitTypeDef gpio = {0};

    /* ---- QSPI peripheral clock & force reset (like reference) ---- */
    __HAL_RCC_QSPI_CLK_ENABLE();
    __HAL_RCC_QSPI_FORCE_RESET();
    __HAL_RCC_QSPI_RELEASE_RESET();

    /* ---- GPIO clocks ---- */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    /* ---- GPIO config ---- */
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_NOPULL;
    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;

    /* PB2 = CLK (AF9) */
    gpio.Alternate = GPIO_AF9_QUADSPI;
    gpio.Pin       = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* PB6 = NCS (AF10) — NOTE: NCS uses AF10, not AF9! */
    gpio.Alternate = GPIO_AF10_QUADSPI;
    gpio.Pin       = GPIO_PIN_6;
    HAL_GPIO_Init(GPIOB, &gpio);

    /* PD11=IO0, PD12=IO1, PD13=IO3 (AF9) */
    gpio.Alternate = GPIO_AF9_QUADSPI;
    gpio.Pin       = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
    HAL_GPIO_Init(GPIOD, &gpio);

    /* PE2 = IO2 (AF9) */
    gpio.Pin       = GPIO_PIN_2;
    HAL_GPIO_Init(GPIOE, &gpio);

    /* ---- QSPI handle init (like reference MX_QUADSPI_Init) ---- */
    p = (uint8_t *)&hqspi;
    for (i = 0; i < sizeof(hqspi); i++) *p++ = 0;

    hqspi.Instance = QUADSPI;
    HAL_QSPI_DeInit(&hqspi);

    hqspi.Init.ClockPrescaler     = 1;    /* 250MHz / (1+1) = 125MHz */
    hqspi.Init.FifoThreshold      = 32;
    hqspi.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE;
    hqspi.Init.FlashSize          = 22;   /* 2^23 = 8MB */
    hqspi.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_1_CYCLE;
    hqspi.Init.ClockMode          = QSPI_CLOCK_MODE_3;
    hqspi.Init.FlashID            = QSPI_FLASH_ID_1;
    hqspi.Init.DualFlash          = QSPI_DUALFLASH_DISABLE;

    if (HAL_QSPI_Init(&hqspi) != HAL_OK) {
        printf("ERR: QSPI Flash init failed!\r\n");
        return;
    }

    /* Chip reset */
    Flash_ResetChip();

    /* Read JEDEC ID */
    id = Flash_ReadID();
    printf("  QSPI Flash: W25Q64JV, JEDEC ID=0x%06lX\r\n", id);

    if (id != 0xEF4017) {
        printf("  WARNING: Unexpected JEDEC ID 0x%06lX (expected 0xEF4017)\r\n", id);
    }
}

int QSPI_Flash_Erase_Sector(uint32_t addr)
{
    QSPI_CommandTypeDef cmd = {0};

    if (Flash_WriteEnable() != 0) {
        printf("ERR: Erase WE fail @0x%06lX\r\n", addr);
        return -1;
    }

    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction       = CMD_SECTOR_ERASE;
    cmd.AddressMode       = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize       = QSPI_ADDRESS_24_BITS;
    cmd.Address           = addr;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_NONE;
    cmd.DummyCycles       = 0;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT) != HAL_OK) {
        printf("ERR: Erase cmd fail @0x%06lX\r\n", addr);
        return -1;
    }
    if (Flash_AutoPollingReady(HAL_QSPI_TIMEOUT) != 0) {
        printf("ERR: Erase timeout @0x%06lX\r\n", addr);
        return -1;
    }
    return 0;
}

int QSPI_Flash_Write_Page(uint32_t addr, const uint8_t *data, uint32_t len)
{
    QSPI_CommandTypeDef cmd = {0};

    if (len > QSPI_PAGE_SIZE) len = QSPI_PAGE_SIZE;

    if (Flash_WriteEnable() != 0) {
        printf("ERR: Write WE fail @0x%06lX\r\n", addr);
        return -1;
    }

    cmd.InstructionMode   = QSPI_INSTRUCTION_1_LINE;
    cmd.Instruction       = CMD_PAGE_PROGRAM;
    cmd.AddressMode       = QSPI_ADDRESS_1_LINE;
    cmd.AddressSize       = QSPI_ADDRESS_24_BITS;
    cmd.Address           = addr;
    cmd.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    cmd.DataMode          = QSPI_DATA_1_LINE;
    cmd.DummyCycles       = 0;
    cmd.NbData            = len;
    cmd.DdrMode           = QSPI_DDR_MODE_DISABLE;
    cmd.DdrHoldHalfCycle  = QSPI_DDR_HHC_ANALOG_DELAY;
    cmd.SIOOMode          = QSPI_SIOO_INST_EVERY_CMD;

    if (HAL_QSPI_Command(&hqspi, &cmd, HAL_QSPI_TIMEOUT) != HAL_OK) {
        printf("ERR: Write cmd fail @0x%06lX\r\n", addr);
        return -1;
    }
    if (HAL_QSPI_Transmit(&hqspi, (uint8_t *)data, HAL_QSPI_TIMEOUT) != HAL_OK) {
        printf("ERR: Write xmit fail @0x%06lX\r\n", addr);
        return -1;
    }
    if (Flash_AutoPollingReady(HAL_QSPI_TIMEOUT) != 0) {
        printf("ERR: Write timeout @0x%06lX\r\n", addr);
        return -1;
    }
    return 0;
}

int QSPI_Flash_Write(uint32_t addr, const uint8_t *data, uint32_t len)
{
    uint32_t remaining = len;
    uint32_t offset = 0;

    while (remaining > 0) {
        uint32_t chunk = (remaining > QSPI_PAGE_SIZE) ? QSPI_PAGE_SIZE : remaining;
        if (QSPI_Flash_Write_Page(addr + offset, data + offset, chunk) != 0)
            return -1;
        offset += chunk;
        remaining -= chunk;
    }
    return 0;
}

int QSPI_Flash_Store_PI(const uint32_t *pi_bignum, int digits,
                         uint32_t *erase_ms, uint32_t *write_ms)
{
    char buf[20];
    int frac_words   = (digits + DIG_PER_WORD - 1) / DIG_PER_WORD;
    int arr_size     = (digits + DIG_PER_WORD - 1) / DIG_PER_WORD + 3;
    uint32_t flash_addr = 0;
    uint32_t total_written = 0;
    uint32_t t_start, cycles_per_ms;
    int hdr_len, int_len, n;
    int i, j;

    cycles_per_ms = SystemCoreClock / 1000U;

    /* Erase first 128KB (32 sectors x 4KB) */
    printf("  Erasing flash (32 sectors)...\r\n");
    t_start = DWT->CYCCNT;
    for (i = 0; i < 32; i++) {
        if (QSPI_Flash_Erase_Sector(i * QSPI_SECTOR_SIZE) != 0) {
            printf("ERR: Erase failed at sector %d\r\n", i);
            return -1;
        }
        if (i % 8 == 7) printf("    erased %d/32 sectors\r\n", i + 1);
    }
    if (erase_ms) *erase_ms = (DWT->CYCCNT - t_start) / cycles_per_ms;
    printf("  Erase done.\r\n");

    /* Header */
    hdr_len = snprintf(buf, sizeof(buf), "PI=%d digits\n", digits);
    if (QSPI_Flash_Write(flash_addr, (uint8_t *)buf, hdr_len) != 0) return -1;
    flash_addr += hdr_len;
    total_written += hdr_len;

    /* Integer part */
    int_len = snprintf(buf, sizeof(buf), "%u.",
                       (unsigned int)pi_bignum[arr_size - 1]);
    if (QSPI_Flash_Write(flash_addr, (uint8_t *)buf, int_len) != 0) return -1;
    flash_addr += int_len;
    total_written += int_len;

    /* Fractional words */
    printf("  Writing PI digits...\r\n");
    t_start = DWT->CYCCNT;
    for (i = 1; i <= frac_words; i++) {
        int idx     = arr_size - 1 - i;
        int is_last = (i == frac_words);
        int digs    = is_last ? (digits - DIG_PER_WORD * (frac_words - 1)) : DIG_PER_WORD;

        if (digs == DIG_PER_WORD) {
            n = snprintf(buf, sizeof(buf), "%09u", (unsigned int)pi_bignum[idx]);
        } else {
            uint32_t pow10 = 1;
            for (j = 0; j < DIG_PER_WORD - digs; j++) pow10 *= 10;
            n = snprintf(buf, sizeof(buf), "%0*u", digs,
                         (unsigned int)(pi_bignum[idx] / pow10));
        }
        if (QSPI_Flash_Write(flash_addr, (uint8_t *)buf, n) != 0) return -1;
        flash_addr += n;
        total_written += n;

        /* Line break every 54 digits */
        if (i % 6 == 0 && i < frac_words) {
            if (QSPI_Flash_Write(flash_addr, (uint8_t *)"\r\n", 2) != 0) return -1;
            flash_addr += 2;
            total_written += 2;
        }
    }

    /* Trailing newline */
    if (QSPI_Flash_Write(flash_addr, (uint8_t *)"\r\n", 2) != 0) return -1;
    total_written += 2;
    if (write_ms) *write_ms = (DWT->CYCCNT - t_start) / cycles_per_ms;

    printf("  Stored %u bytes to QSPI Flash\r\n", (unsigned int)total_written);
    return total_written;
}
