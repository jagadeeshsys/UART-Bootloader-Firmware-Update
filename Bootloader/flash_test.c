
#include "uart.h"
#include "flash_mock.h"

#define PCLK_HZ       15000000U
#define BAUD_RATE     115200U

int main(void)
{
    uint8_t firmware[] =
    {
        0x11, 0x22, 0x33, 0x44,
        0x55, 0x66, 0x77, 0x88
    };

    uint8_t read_buffer[8];

    int result;

    UART0_Init(PCLK_HZ, BAUD_RATE);

    UART0_TxString("\r\n");
    UART0_TxString("LPC2129 FLASH SIMULATION TEST\r\n");

    FlashMock_Init();

    UART0_TxString("\r\nTEST 1: ERASE SECTOR\r\n");

    result = FlashMock_EraseSector(0);

    if (result == FLASH_OK)
        UART0_TxString("PASS: Sector erased\r\n");
    else
        UART0_TxString("FAIL: Sector erase\r\n");

    UART0_TxString("\r\nTEST 2: WRITE FIRMWARE\r\n");

    result = FlashMock_Write(
        0,
        firmware,
        sizeof(firmware)
    );

    if (result == FLASH_OK)
        UART0_TxString("PASS: Firmware written\r\n");
    else
        UART0_TxString("FAIL: Firmware write\r\n");

    UART0_TxString("\r\nTEST 3: READ FIRMWARE\r\n");

    result = FlashMock_Read(
        0,
        read_buffer,
        sizeof(read_buffer)
    );

    if (result == FLASH_OK)
        UART0_TxString("PASS: Firmware read\r\n");
    else
        UART0_TxString("FAIL: Firmware read\r\n");

    UART0_TxString("\r\nTEST 4: VERIFY FIRMWARE\r\n");

    result = FlashMock_Verify(
        0,
        firmware,
        sizeof(firmware)
    );

    if (result == FLASH_OK)
        UART0_TxString("PASS: Firmware verified\r\n");
    else
        UART0_TxString("FAIL: Firmware verification\r\n");

    UART0_TxString("\r\nFLASH TEST COMPLETED\r\n");

    while (1)
    {
    }
}
