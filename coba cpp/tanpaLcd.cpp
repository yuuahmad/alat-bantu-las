#include <Arduino.h>
/* @file HelloKeypad.pde
|| @version 1.0
|| @author Alexander Brevig
|| @contact alexanderbrevig@gmail.com
||
|| @description
|| | Demonstrates the simplest use of the matrix Keypad library.
|| #
*/
#include <Keypad.h>
#include <Wire.h>

const byte ROWS = 2; // four rows
const byte COLS = 4; // three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3', '4'},
    {'A', 'B', 'C', 'D'}};
byte rowPins[ROWS] = {6, 7};       // connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4, 5}; // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// semua variabel tentang stepper motor
const int stepPin = 11;
float stepPinState = LOW;      // low=relase mode, high=hold mode
unsigned long stepPinInterval; // interval atau lama step pin mati/nyala
const float dirPin = 12;
float pulsePerRevolusi = 3200; // step per revolusi (ada pada stepper driver)
bool hold = false;

// variabel2 yang penting untuk tampilan layar dan letak variabel
int nilaiMenu = 0;

// variabel penting / utama
float kecepatanPutar = 60;  // derajat per detik */s
float percepatanPutar = 10; // derajat per detik kuadrat */s^2
bool arah = true;           // true artinya cw dan false artinya ccw
bool run = false;           // true artinya run, dan false artinya stop
float sleep = 600;          // 600 detik (ke mode sleep setelah 600 detik)
float derajatBenda = 0;     // nilai derajat.
float lompatanSudut = 10;   // setiap melompat sejauh 45 derajat (ganti ke eeprom nanti)
bool lompat = false;
int hitungStepLompat = 0; // untuk menghitung sudah lompatan

// variabel2 advance dan kalkukasi
unsigned long previousMicros = 0;  // will store last time LED was updated
unsigned long previousMicros2 = 0; // will store last time LED was updated
unsigned long previousMicros3 = 0; // will store last time LED was updated

void setup()
{
    Serial.begin(115200);
}

void loop()
{
    // waktu adalah dewa
    // semua rumus perhitungan dihitung dulu sebelum dieksekusi
    // satuan stepPinInterval adalah step/detik v
    // kita jadikan sebagai acuan untuk kecepatan dan percepatan
    // stepPinInterval = (1000000 / pulsePerRevolusi) / 2;                    // 1detik(1000milidetik)/pulsePerRevolusi/2(satu cycle=1low,1high)

    stepPinInterval = 1000;
    int nilaiStepLompatanSudut = (lompatanSudut / 360) * pulsePerRevolusi; // kedepan

    // semua program tentang fungsi tombol di tampilan utama ada disini
    char key = keypad.getKey();

    if (key == '1') // tombol menu ditekan. pergi ke menu
    {
        nilaiMenu++;
        if (nilaiMenu > 3)
            nilaiMenu = 1;
    }

    if (key == '4')
        nilaiMenu = 0;

    if (key == 'A') // tombol arah ditekan. ganti cw atau ccw
        arah = !arah;

    if (key == 'B') // jika tombol run ditekan, jalankan atau hentikan motor
        run = !run;

    if (key == 'C') // tombol lompat ditekan, lompat sekarang
        lompat = true;

    if (key == 'D') // tombol hold ditekan, hold atau relase motor
        hold = !hold;

    // program untuk menjalankan dan menghentikan stepper secara kontinu, atau semuanya? :3
    // program untuk melakukan hold motor
    unsigned long currentMicros = micros();
    if (run == false && lompat == false)
        if (hold == true)
            digitalWrite(stepPin, HIGH);
        else
            digitalWrite(stepPin, LOW);
    else if (currentMicros - previousMicros >= stepPinInterval && (run == true || lompat == true))
    {
        previousMicros = currentMicros;
        if (arah == true)
            digitalWrite(dirPin, HIGH);
        else
            digitalWrite(dirPin, LOW);
        if (stepPinState == LOW)
            stepPinState = HIGH;
        else
            stepPinState = LOW;
        digitalWrite(stepPin, stepPinState);
        if (lompat == true)
            hitungStepLompat++;
        if (hitungStepLompat > nilaiStepLompatanSudut)
        {
            hitungStepLompat = 0;
            lompat = false;
        }
    }

    // jika menu > 1 maka tampilkan beberapa variabel yang dapat diubah
    //  jika menu == 0 maka tampilkan menu utama
    // Gunakan switch case untuk menentukan aksi berdasarkan nilai sensor
    switch (nilaiMenu)
    {
    case 0:
        Serial.println("UTAMA");
        // proogram menampilkan ui baris pertama

        // proogram menampilkan ui baris kedua
        if (arah == true)
        {
            Serial.println("CW");
        }
        else
        {
            Serial.println("CCW");
        }
        if (run == true)
        {
            Serial.println("RUN");
        }
        else
        {
            Serial.println("STOP");
        }
        if (lompat == true)
        {
            Serial.println("LOMPAT");
        }
        else
        {
            Serial.println("JANGKAR");
        }
        if (hold == true)
        {
            Serial.println("H");
        }
        else
        {
            Serial.println("R");
        }
        break;
    case 1:
        if (key == '2')
            kecepatanPutar--;
        if (key == '3')
            kecepatanPutar++;
        // Tambahkan tindakan yang sesuai untuk nilai sedang
        break;
    case 2:
        if (key == '2')
            percepatanPutar--;
        if (key == '3')
            percepatanPutar++;
        // Tambahkan tindakan yang sesuai untuk nilai tinggi
        break;
    case 3:
        if (key == '2')
            lompatanSudut--;
        if (key == '3')
            lompatanSudut++;
        // Tambahkan tindakan yang sesuai untuk nilai tinggi
        break;
    default:
        Serial.print("ERROR: Unknown");
        // proogram menampilkan ui baris pertama
        // Tambahkan tindakan default untuk nilai di luar rentang yang diharapkan
    }

    if (key)
    {
        Serial.print("key=");
        Serial.println(key);
        Serial.print(" nil.menu=");
        Serial.println(nilaiMenu);
        Serial.print(" kec.putar=");
        Serial.println(kecepatanPutar);
        Serial.print(" per.putar=");
        Serial.println(percepatanPutar);
        Serial.print(" lom.sudut=");
        Serial.println(lompatanSudut);
        Serial.print(" lompat=");
        Serial.println(lompat);
        Serial.print(" run=");
        Serial.println(run);
    }
}
