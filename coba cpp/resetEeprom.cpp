#include <EEPROM.h>
#include <Arduino.h>

void resetEEPROM()
{
    for (int i = 0; i < EEPROM.length(); i++)
    {
        EEPROM.write(i, 0);
    }
}

void setup()
{
    // Panggil fungsi resetEEPROM() saat program dijalankan
    resetEEPROM();

    // Tambahkan kode setup lainnya sesuai kebutuhan
}

void loop()
{
    // Program utama, tambahkan kode loop sesuai kebutuhan
}
