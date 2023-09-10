#include "settings.h"
#include <assert.h>
#include "stm32f3xx_hal.h"

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
    uint32_t wordAddress = SETTINGS_FLASH_ADDRESS;
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
    settingsRegMap[SETTINGS_REG_PTT1] = SETTINGS_REG_PTT1_DEFAULT;
    settingsRegMap[SETTINGS_REG_PTT2] = SETTINGS_REG_PTT2_DEFAULT;
}
