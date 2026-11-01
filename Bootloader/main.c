#include "bootloader.h"

int main(void)
{
    Bootloader_Init();

    while (1)
    {
        Bootloader_Process();
    }
}