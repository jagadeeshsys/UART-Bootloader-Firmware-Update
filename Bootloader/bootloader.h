
#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>

/* Bootloader states */

#define BL_STATE_IDLE          0U
#define BL_STATE_RECEIVING     1U
#define BL_STATE_VERIFYING     2U
#define BL_STATE_COMPLETE      3U
#define BL_STATE_ERROR         4U

/* Bootloader status */

#define BL_OK                  0
#define BL_ERROR              -1

void Bootloader_Init(void);

void Bootloader_Process(void);

uint8_t Bootloader_GetState(void);
void Bootloader_TestInjectByte(uint8_t byte);

#endif
