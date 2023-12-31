#include "settings.h"
#include <assert.h>
#include "stm32f3xx_hal.h"

/* Define this, so that each time the AIOC is programmed, the EEPROM is cleared.
 * This is inconvenient, but settings might not be portable across versions. */
uint32_t settingsRegMapROM[SETTINGS_REGMAP_SIZE] __attribute__ ((section (".eeprom"))) = {[0 ... (SETTINGS_REGMAP_SIZE-1)] = 0xFF};

uint32_t settingsRegMap[SETTINGS_REGMAP_SIZE] = {[0 ... (SETTINGS_REGMAP_SIZE-1)] = 0};

void Settings_Init()
{
    Settings_Recall();
}

uint8_t Settings_RegWrite(uint8_t address, const uint8_t * buffer, uint8_t bufsize)
{
    if (address < SETTINGS_REGMAP_SIZE) {
        assert(bufsize == sizeof(uint32_t));

        uint32_t data = (   (((uint32_t) buffer[0]) << 0) |
                            (((uint32_t) buffer[1]) << 8) |
                            (((uint32_t) buffer[2]) << 16) |
                            (((uint32_t) buffer[3]) << 24) );

        settingsRegMap[address] = data;

        return bufsize;
    }

    return 0;
}

uint8_t Settings_RegRead(uint8_t address, uint8_t * buffer, uint8_t bufsize)
{
    if (address < SETTINGS_REGMAP_SIZE) {
        assert(bufsize == sizeof(uint32_t));

        uint64_t data = settingsRegMap[address];

        buffer[0] = (uint8_t) (data >> 0);
        buffer[1] = (uint8_t) (data >> 8);
        buffer[2] = (uint8_t) (data >> 16);
        buffer[3] = (uint8_t) (data >> 24);

        return bufsize;
    }
}

void Settings_Store(void)
{
    HAL_FLASH_Unlock();

    uint32_t pageError = 0;
    FLASH_EraseInitTypeDef eraseInitStruct = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .PageAddress = SETTINGS_FLASH_ADDRESS,
        .NbPages = (SETTINGS_REGMAP_SIZE * sizeof(uint32_t) + FLASH_PAGE_SIZE-1) / FLASH_PAGE_SIZE
    };


    if (HAL_FLASHEx_Erase(&eraseInitStruct, &pageError) != HAL_OK) {
        uint32_t flashError = HAL_FLASH_GetError();
        return;
    }

    uint32_t wordAddress = SETTINGS_FLASH_ADDRESS;
    uint32_t wordCount = SETTINGS_REGMAP_SIZE;
    uint32_t * wordPtr = settingsRegMap;

    while (wordCount--) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, wordAddress, *wordPtr) != HAL_OK) {
            uint32_t flashError = HAL_FLASH_GetError();
            return;
        }

        wordPtr++;
        wordAddress += sizeof(uint32_t);
    }

    HAL_FLASH_Lock();
}

void Settings_Recall(void)
{
    /* From linker script */
    extern uint32_t * _eeprom;

    uint32_t wordAddress = (uint32_t) &_eeprom;
    uint32_t wordCount = SETTINGS_REGMAP_SIZE;
    uint32_t * wordPtr = settingsRegMap;

    uint32_t magic = (*(__IO uint32_t *) wordAddress);

    if ( magic == SETTINGS_REG_MAGIC_DEFAULT ) {
        while (wordCount--) {
            *wordPtr++ = *(__IO uint32_t *) wordAddress;
            wordAddress += sizeof(uint32_t);
        }
    } else {
        /* Magic token not found, assume flash is unprogrammed */
        Settings_Default();
    }

}

void Settings_Default(void)
{
    settingsRegMap[SETTINGS_REG_MAGIC] = SETTINGS_REG_MAGIC_DEFAULT;
    settingsRegMap[SETTINGS_REG_USBID] = SETTINGS_REG_USBID_DEFAULT;

    /* AIOC registers */
    settingsRegMap[SETTINGS_REG_AIOC_IOMUX0] = SETTINGS_REG_AIOC_IOMUX0_DEFAULT;
    settingsRegMap[SETTINGS_REG_AIOC_IOMUX1] = SETTINGS_REG_AIOC_IOMUX1_DEFAULT;

    /* CM108 registers */
    settingsRegMap[SETTINGS_REG_CM108_IOMUX0] = SETTINGS_REG_CM108_IOMUX0_DEFAULT;
    settingsRegMap[SETTINGS_REG_CM108_IOMUX1] = SETTINGS_REG_CM108_IOMUX1_DEFAULT;
    settingsRegMap[SETTINGS_REG_CM108_IOMUX2] = SETTINGS_REG_CM108_IOMUX2_DEFAULT;
    settingsRegMap[SETTINGS_REG_CM108_IOMUX3] = SETTINGS_REG_CM108_IOMUX3_DEFAULT;

    /* Serial (CDC) registers */
    settingsRegMap[SETTINGS_REG_SERIAL_CTRL] = SETTINGS_REG_SERIAL_CTRL_DEFAULT;
    settingsRegMap[SETTINGS_REG_SERIAL_IOMUX0] = SETTINGS_REG_SERIAL_IOMUX0_DEFAULT;
    settingsRegMap[SETTINGS_REG_SERIAL_IOMUX1] = SETTINGS_REG_SERIAL_IOMUX1_DEFAULT;
    settingsRegMap[SETTINGS_REG_SERIAL_IOMUX2] = SETTINGS_REG_SERIAL_IOMUX2_DEFAULT;
    settingsRegMap[SETTINGS_REG_SERIAL_IOMUX3] = SETTINGS_REG_SERIAL_IOMUX3_DEFAULT;

    /* Virtual PTT registers */
    settingsRegMap[SETTINGS_REG_VPTT_LVLCTRL] = SETTINGS_REG_VPTT_LVLCTRL_DEFAULT;
    settingsRegMap[SETTINGS_REG_VPTT_TIMCTRL] = SETTINGS_REG_VPTT_TIMCTRL_DEFAULT;

    /* Virtual COS registers */
    settingsRegMap[SETTINGS_REG_VCOS_LVLCTRL] = SETTINGS_REG_VCOS_LVLCTRL_DEFAULT;
    settingsRegMap[SETTINGS_REG_VCOS_TIMCTRL] = SETTINGS_REG_VCOS_TIMCTRL_DEFAULT;


}
