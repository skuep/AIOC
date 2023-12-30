#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <stdint.h>
#include "usb_descriptors.h"

#define SETTINGS_REGMAP_SIZE     256
#define SETTINGS_FLASH_ADDRESS   0x0801F000UL

extern uint32_t settingsRegMap[SETTINGS_REGMAP_SIZE];

/* Magic number register. Mainly used to see if flash data is valid */
#define SETTINGS_REG_MAGIC                          0x00
#define SETTINGS_REG_MAGIC_DEFAULT                  ( (((uint32_t) 'A') <<  0) | \
                                                      (((uint32_t) 'I') <<  8) | \
                                                      (((uint32_t) 'O') << 16) | \
                                                      (((uint32_t) 'C') << 24) )

/* USB ID register. The default USB VID and PID can be overwritten. Use with caution */
#define SETTINGS_REG_USBID                          0x08
#define SETTINGS_REG_USBID_DEFAULT                  ( (((uint32_t)(USB_VID & SETTINGS_REG_USBID_VID_MASK)) << SETTINGS_REG_USBID_VID_OFFS) | \
                                                      (((uint32_t)(USB_PID & SETTINGS_REG_USBID_PID_MASK)) << SETTINGS_REG_USBID_PID_OFFS) )
#define SETTINGS_REG_USBID_VID_OFFS                 0
#define SETTINGS_REG_USBID_VID_MASK                 0xFFFFUL
#define SETTINGS_REG_USBID_PID_OFFS                 16
#define SETTINGS_REG_USBID_PID_MASK                 0xFFFFUL

/* PTT1 register. Allows configuration on when to assert PTT1 */
#define SETTINGS_REG_PTT1                           0x10
#define SETTINGS_REG_PTT1_DEFAULT                   (SETTINGS_REG_PTT1_SRC_CM108GPIO3_MASK | SETTINGS_REG_PTT1_SRC_SERIALDTRNRTS_MASK)
#define SETTINGS_REG_PTT1_SRC_CM108GPIO1_MASK       0x00000100UL
#define SETTINGS_REG_PTT1_SRC_CM108GPIO2_MASK       0x00000200UL
#define SETTINGS_REG_PTT1_SRC_CM108GPIO3_MASK       0x00000400UL
#define SETTINGS_REG_PTT1_SRC_CM108GPIO4_MASK       0x00000800UL
#define SETTINGS_REG_PTT1_SRC_SERIALDTR_MASK        0x00010000UL
#define SETTINGS_REG_PTT1_SRC_SERIALRTS_MASK        0x00020000UL
#define SETTINGS_REG_PTT1_SRC_SERIALDTRNRTS_MASK    0x00040000UL
#define SETTINGS_REG_PTT1_SRC_SERIALNDTRRTS_MASK    0x00080000UL

/* PTT2 register. Allows configuration on when to assert PTT2 */
#define SETTINGS_REG_PTT2                           0x11
#define SETTINGS_REG_PTT2_DEFAULT                   (SETTINGS_REG_PTT2_SRC_CM108GPIO4_MASK)
#define SETTINGS_REG_PTT2_SRC_CM108GPIO1_MASK       0x00000100ULL
#define SETTINGS_REG_PTT2_SRC_CM108GPIO2_MASK       0x00000200ULL
#define SETTINGS_REG_PTT2_SRC_CM108GPIO3_MASK       0x00000400ULL
#define SETTINGS_REG_PTT2_SRC_CM108GPIO4_MASK       0x00000800ULL
#define SETTINGS_REG_PTT2_SRC_SERIALDTR_MASK        0x00010000ULL
#define SETTINGS_REG_PTT2_SRC_SERIALRTS_MASK        0x00020000ULL
#define SETTINGS_REG_PTT2_SRC_SERIALDTRNRTS_MASK    0x00040000ULL
#define SETTINGS_REG_PTT2_SRC_SERIALNDTRRTS_MASK    0x00080000ULL

void Settings_Init();
uint8_t Settings_RegWrite(uint8_t address, const uint8_t * buffer, uint8_t bufsize);
uint8_t Settings_RegRead(uint8_t address, uint8_t * buffer, uint8_t bufsize);
void Settings_Store(void);
void Settings_Recall(void);
void Settings_Default(void);

#endif /* SETTINGS_H_ */
