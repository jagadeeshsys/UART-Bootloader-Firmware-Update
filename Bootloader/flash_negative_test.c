
#include "uart.h"
#include "flash_mock.h"

#define PCLK_HZ       15000000U
#define BAUD_RATE     115200U

int main(void)
{
    uint8_t firmware[] =
    {
        0x11, 0x22, 0x33, 0x44
    };

    uint8_t invalid_data[] =
    {
        0xFF, 0xFF, 0xFF, 0xFF
    };

    int result;

    UART0_Init(PCLK_HZ, BAUD_RATE);

    FlashMock_Init();

    UART0_TxString("\r\n");
    UART0_TxString("===============================\r\n");
    UART0_TxString("FLASH NEGATIVE TESTING\r\n");
    UART0_TxString("===============================\r\n");

    /* TEST 1: Invalid sector */

    UART0_TxString("\r\nTEST 1: INVALID SECTOR\r\n");

    result = FlashMock_EraseSector(10);

    if (result == FLASH_ERROR)
        UART0_TxString("PASS: Invalid sector rejected\r\n");
    else
        UART0_TxString("FAIL: Invalid sector accepted\r\n");

    /* TEST 2: Initial firmware write */

    UART0_TxString("\r\nTEST 2: INITIAL WRITE\r\n");

    result = FlashMock_Write(
        0,
        firmware,
        sizeof(firmware)
    );

    if (result == FLASH_OK)
        UART0_TxString("PASS: Firmware written\r\n");
    else
        UART0_TxString("FAIL: Firmware write failed\r\n");

    /* TEST 3: Illegal 0-to-1 programming */

    UART0_TxString("\r\nTEST 3: ILLEGAL BIT TRANSITION\r\n");

    result = FlashMock_Write(
        0,
        invalid_data,
        sizeof(invalid_data)
    );

    if (result == FLASH_ERROR)
        UART0_TxString("PASS: Illegal write rejected\r\n");
    else
        UART0_TxString("FAIL: Illegal write accepted\r\n");

    /* TEST 4: Out-of-bounds write */

    UART0_TxString("\r\nTEST 4: OUT OF BOUNDS WRITE\r\n");

    result = FlashMock_Write(
        MOCK_FLASH_SIZE - 2U,
        firmware,
        sizeof(firmware)
    );

    if (result == FLASH_ERROR)
        UART0_TxString("PASS: Invalid address rejected\r\n");
    else
        UART0_TxString("FAIL: Invalid address accepted\r\n");

    /* TEST 5: Erase and rewrite */

    UART0_TxString("\r\nTEST 5: ERASE AND REWRITE\r\n");

    result = FlashMock_EraseSector(0);

    if (result == FLASH_OK)
    {
        result = FlashMock_Write(
            0,
            invalid_data,
            sizeof(invalid_data)
        );
    }

    if (result == FLASH_OK)
        UART0_TxString("PASS: Rewrite successful\r\n");
    else
        UART0_TxString("FAIL: Rewrite failed\r\n");

    UART0_TxString("\r\n===============================\r\n");
    UART0_TxString("NEGATIVE TESTING COMPLETED\r\n");
    UART0_TxString("===============================\r\n");

    while (1)
    {
    }
}
