
#ifndef FLASH_MOCK_H
#define FLASH_MOCK_H

#include <stdint.h>


#define MOCK_FLASH_SIZE    4096U
#define MOCK_SECTOR_SIZE   1024U
#define MOCK_SECTOR_COUNT  4U


#define FLASH_OK           0
#define FLASH_ERROR       -1

void FlashMock_Init(void);

int FlashMock_EraseSector(uint32_t sector);

int FlashMock_Write(
    uint32_t offset,
    const uint8_t *data,
    uint32_t length
);

int FlashMock_Read(
    uint32_t offset,
    uint8_t *data,
    uint32_t length
);

int FlashMock_Verify(
    uint32_t offset,
    const uint8_t *data,
    uint32_t length
);

#endif
