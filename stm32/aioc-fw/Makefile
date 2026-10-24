TARGET = aioc-fw
BUILD_DIR = build

PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size

MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard

# 1. Поиск C-файлов (исключаем Templates, *_template.c и чужие драйверы TinyUSB)
RAW_C_SOURCES = $(shell find Src Drivers Middlewares/Third-Party/tinyusb/src -name "*.c" 2>/dev/null)

C_SOURCES = $(foreach file, $(RAW_C_SOURCES), \
    $(if $(findstring /Templates/,$(file)),, \
    $(if $(findstring /portable/,$(file)), \
        $(if $(findstring /portable/st/,$(file)),$(file), \
        $(if $(findstring /portable/synopsys/,$(file)),$(file),)), \
    $(file))))
C_SOURCES := $(filter-out %_template.c, $(C_SOURCES))
C_SOURCES := $(sort $(C_SOURCES))

# 2. Поиск ASM-файлов (исключаем Templates, Keil /arm/ и IAR /iar/)
RAW_ASM_SOURCES = $(shell find Src Drivers -name "*.s" -o -name "*.S" 2>/dev/null)

ASM_SOURCES = $(foreach file, $(RAW_ASM_SOURCES), \
    $(if $(findstring /Templates/,$(file)),, \
    $(if $(findstring /arm/,$(file)),, \
    $(if $(findstring /iar/,$(file)),, \
    $(file)))))
ASM_SOURCES := $(sort $(ASM_SOURCES))

# 3. Директории с заголовками .h
INC_DIRS = $(sort $(dir $(shell find Inc Src Drivers Middlewares/Third-Party/tinyusb/src -name "*.h" 2>/dev/null)))
C_INCLUDES = $(addprefix -I,$(INC_DIRS))

# Дефайны для STM32 HAL и TinyUSB
C_DEFS = \
-DSTM32F302xC \
-DUSE_HAL_DRIVER \
-DCFG_TUSB_MCU=OPT_MCU_STM32F3

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) -O2 -Wall -fdata-sections -ffunction-sections

# Скрипт линковщика и флаги
LDSCRIPT = stm32f30_flash.ld
LDFLAGS = $(MCU) -T$(LDSCRIPT) -Wl,--gc-sections -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -lm -lc -lnosys

# Объекты
C_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(C_SOURCES))
ASM_OBJS = $(patsubst %.s, $(BUILD_DIR)/%.o, $(patsubst %.S, $(BUILD_DIR)/%.o, $(ASM_SOURCES)))
OBJECTS = $(C_OBJS) $(ASM_OBJS)

all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).bin

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.S
	@mkdir -p $(dir $@)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf
	$(CP) -O binary $< $@

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean
