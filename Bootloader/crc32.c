
#include "crc32.h"

#define CRC32_POLYNOMIAL  0xEDB88320UL

uint32_t CRC32_Calculate(const uint8_t *data, uint32_t length)
{
    uint32_t crc;
    uint32_t i;
    uint32_t j;

    crc = 0xFFFFFFFFUL;

    for (i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (j = 0; j < 8; j++)
        {
            if (crc & 1UL)
            {
                crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return (crc ^ 0xFFFFFFFFUL);
}
