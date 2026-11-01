
#include "flash_mock.h"

/* Simulated flash memory */

static uint8_t mock_flash[MOCK_FLASH_SIZE];

/* Initialize flash to erased state */

void FlashMock_Init(void)
{
    uint32_t i;

    for (i = 0; i < MOCK_FLASH_SIZE; i++)
    {
        mock_flash[i] = 0xFF;
    }
}

/* Erase one sector */

int FlashMock_EraseSector(uint32_t sector)
{
    uint32_t start;
    uint32_t i;

    if (sector >= MOCK_SECTOR_COUNT)
        return FLASH_ERROR;

    start = sector * MOCK_SECTOR_SIZE;

    for (i = 0; i < MOCK_SECTOR_SIZE; i++)
    {
        mock_flash[start + i] = 0xFF;
    }

    return FLASH_OK;
}

/* Program flash memory */

int FlashMock_Write(
    uint32_t offset,
    const uint8_t *data,
    uint32_t length)
{
    uint32_t i;

    if (data == 0)
        return FLASH_ERROR;

    if (offset > MOCK_FLASH_SIZE ||
        length > MOCK_FLASH_SIZE - offset)
        return FLASH_ERROR;

    /* Check all bytes before programming */

    for (i = 0; i < length; i++)
    {
        if ((mock_flash[offset + i] & data[i]) != data[i])
        {
            return FLASH_ERROR;
        }
    }

    /* Flash programming: 1 -> 0 */

    for (i = 0; i < length; i++)
    {
        mock_flash[offset + i] &= data[i];
    }

    return FLASH_OK;
}

/* Read flash memory */

int FlashMock_Read(
    uint32_t offset,
    uint8_t *data,
    uint32_t length)
{
    uint32_t i;

    if (data == 0)
        return FLASH_ERROR;

    if (offset > MOCK_FLASH_SIZE ||
        length > MOCK_FLASH_SIZE - offset)
        return FLASH_ERROR;

    for (i = 0; i < length; i++)
    {
        data[i] = mock_flash[offset + i];
    }

    return FLASH_OK;
}

/* Verify programmed data */

int FlashMock_Verify(
    uint32_t offset,
    const uint8_t *data,
    uint32_t length)
{
    uint32_t i;

    if (data == 0)
        return FLASH_ERROR;

    if (offset > MOCK_FLASH_SIZE ||
        length > MOCK_FLASH_SIZE - offset)
        return FLASH_ERROR;

    for (i = 0; i < length; i++)
    {
        if (mock_flash[offset + i] != data[i])
        {
            return FLASH_ERROR;
        }
    }

    return FLASH_OK;
}
