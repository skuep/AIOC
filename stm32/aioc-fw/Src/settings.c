#include "settings.h"
#include <assert.h>
#include "stm32f3xx_hal.h"

/* Define this, so that each time the AIOC is programmed, the EEPROM is cleared.
 * This is inconvenient, but settings might not be portable across versions. */
uint32_t settingsRegMapROM[SETTINGS_REGMAP_SIZE] __attribute__ ((section (".eeprom"))) = {[0 ... (SETTINGS_REGMAP_SIZE-1)] = 0xFFFFFFFFUL};

uint32_t settingsRegMap[SETTINGS_REGMAP_SIZE] = {[0 ... (SETTINGS_REGMAP_SIZE-1)] = 0};

void Settings_Init()
{
    Settings_Recall();
}

uint8_t Settings_RegWrite(uint8_t address, uint32_t data)
{
    if (address < SETTINGS_REGMAP_READONLYADDR) {
        __disable_irq();
        settingsRegMap[address] = data;
        __enable_irq();

        return 1;
    }

    return 0;
}

uint8_t Settings_RegRead(uint8_t address, uint32_t * data)
{
    if (address < SETTINGS_REGMAP_SIZE) {
        __disable_irq();
        *data = settingsRegMap[address];
        __enable_irq();

        return 1;
    }

    return 0;
}

void Settings_Store(void)
{
    HAL_FLASH_Unlock();

    uint32_t pageError = 0;
    extern uint32_t * _eeprom;
    uint32_t pageAddress = (uint32_t) &_eeprom;

    FLASH_EraseInitTypeDef eraseInitStruct = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .PageAddress = pageAddress,
        .NbPages = (SETTINGS_REGMAP_SIZE * sizeof(uint32_t) + FLASH_PAGE_SIZE-1) / FLASH_PAGE_SIZE
    };


    if (HAL_FLASHEx_Erase(&eraseInitStruct, &pageError) != HAL_OK) {
        uint32_t flashError = HAL_FLASH_GetError();
        return;
    }

    extern uint32_t * _eeprom;
    uint32_t wordAddress = (uint32_t) &_eeprom;
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

    /* AIOC Debug registers */
    settingsRegMap[SETTINGS_REG_INFO_AIOC0] = SETTINGS_REG_INFO_AIOC0_DEFAULT;

    /* Audio Debug registers */
    settingsRegMap[SETTINGS_REG_INFO_AUDIO0] = SETTINGS_REG_INFO_AUDIO0_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO1] = SETTINGS_REG_INFO_AUDIO1_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO2] = SETTINGS_REG_INFO_AUDIO2_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO3] = SETTINGS_REG_INFO_AUDIO3_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO4] = SETTINGS_REG_INFO_AUDIO4_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO5] = SETTINGS_REG_INFO_AUDIO5_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO6] = SETTINGS_REG_INFO_AUDIO6_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO7] = SETTINGS_REG_INFO_AUDIO7_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO8] = SETTINGS_REG_INFO_AUDIO9_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO9] = SETTINGS_REG_INFO_AUDIO10_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO10] = SETTINGS_REG_INFO_AUDIO11_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO11] = SETTINGS_REG_INFO_AUDIO11_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO12] = SETTINGS_REG_INFO_AUDIO12_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO13] = SETTINGS_REG_INFO_AUDIO13_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO14] = SETTINGS_REG_INFO_AUDIO14_DEFAULT;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO15] = SETTINGS_REG_INFO_AUDIO15_DEFAULT;

}
