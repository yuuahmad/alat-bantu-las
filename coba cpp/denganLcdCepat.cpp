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
#include <LCD_I2C.h>
// #include <LCD_I2C-C.h>
#include <Wire.h>
// we have a 16 char, 2 line display at address 027 hex
#define MAX_LINES 2
#define MAX_CHARS 16
#define LCD_ADDRESS 0x27

// The I2C bus speed. 100000 is the default, but if your display supports it,
// You can try 400000 for faster response. (Though your eye won't notice.)
#define WIRESPEED 3400000
// LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
LCD_I2C lcd = LCD_I2C(LCD_ADDRESS, MAX_CHARS, MAX_LINES);

byte ikonDerajatPerDetik[8] = {
    0b10010,
    0b00100,
    0b01000,
    0b10011,
    0b00100,
    0b00010,
    0b00001,
    0b00110};

byte ikonDerajatPerDetik2[8] = {
    0b10010,
    0b00100,
    0b01001,
    0b10011,
    0b00100,
    0b00010,
    0b00001,
    0b00110};

byte ikonSekat[8] = {
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100};

byte ikonDerajat[8] = {
    0b11000,
    0b10000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000,
    0b00000};

byte ikonLompat[8] = {
    0b01110,
    0b10001,
    0b10001,
    0b10001,
    0b10001,
    0b10101,
    0b10011,
    0b10001};

byte ikonPijak[8] = {
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b00100,
    0b11111};

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
    // Serial.begin(9600);
    Wire.setClock(WIRESPEED);
    lcd.begin(); // initialize the lcd
    // Print a message to the LCD
    lcd.backlight();
    lcd.createChar(0, ikonDerajat);          // ikon home
    lcd.createChar(1, ikonDerajatPerDetik);  // ikon derajat per detik
    lcd.createChar(2, ikonDerajatPerDetik2); // ikon derajat per detik2
    lcd.createChar(3, ikonSekat);            // ikon sekat
    lcd.createChar(4, ikonLompat);           // ikon lompat
    lcd.createChar(5, ikonPijak);            // ikon pijak

    // tampilan inisialisasi

    // setup awal (perhitungan semua off)
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
        // Serial.print("UTAMA");
        // proogram menampilkan ui baris pertama
        lcd.setCursor(0, 0);
        lcd.print(kecepatanPutar, 0);
        lcd.write(byte(1)); // derajat per detik
        lcd.write(byte(3)); // sekat
        lcd.print(percepatanPutar, 0);
        lcd.write(byte(2)); // derajat per detik2
        lcd.write(byte(3)); // sekat
        lcd.print("x");
        lcd.print(lompatanSudut, 0);
        lcd.write(byte(0)); // derajat
        lcd.print("               ");
        // proogram menampilkan ui baris kedua
        lcd.setCursor(0, 1);
        if (arah == true)
        {
            // Serial.print("CW");
            lcd.print("CW");
        }
        else
        {
            // Serial.print("CCW");
            lcd.print("CCW");
        }
        lcd.write(byte(3)); // sekat
        if (run == true)
        {
            lcd.print("RUN");
            // Serial.print("RUN");
        }
        else
        {
            // Serial.print("STOP");
            lcd.print("STOP");
        }
        lcd.write(byte(3)); // sekat
        if (lompat == true)
        {
            // Serial.print("LOMPAT");
            lcd.write(byte(4));
        }
        else
        {
            // Serial.print("JANGKAR");
            lcd.write(byte(5));
        }
        lcd.write(byte(3)); // sekat
        if (hold == true)
        {
            // Serial.print("H");
            lcd.print("H");
        }
        else
        {
            // Serial.print("R");
            lcd.print("R");
        }
        lcd.print("               ");
        break;
    case 1:
        lcd.setCursor(0, 0);
        lcd.print("kec. putar      ");
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.print(kecepatanPutar, 2);
        lcd.print("                ");
        if (key == '2')
            kecepatanPutar--;
        if (key == '3')
            kecepatanPutar++;
        // Tambahkan tindakan yang sesuai untuk nilai sedang
        break;
    case 2:
        lcd.setCursor(0, 0);
        lcd.print("percep. putar   ");
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.print(percepatanPutar, 2);
        lcd.print("                ");
        if (key == '2')
            percepatanPutar--;
        if (key == '3')
            percepatanPutar++;
        // Tambahkan tindakan yang sesuai untuk nilai tinggi
        break;
    case 3:
        lcd.setCursor(0, 0);
        lcd.print("lompatan sudut  ");
        lcd.setCursor(0, 1);
        lcd.print(" ");
        lcd.print(lompatanSudut, 2);
        lcd.print("                ");
        if (key == '2')
            lompatanSudut--;
        if (key == '3')
            lompatanSudut++;
        // Tambahkan tindakan yang sesuai untuk nilai tinggi
        break;
    default:
        // Serial.print("ERROR: Unknown");
        // proogram menampilkan ui baris pertama
        lcd.setCursor(0, 0);
        lcd.print("ERROR!!?!       ");
        lcd.setCursor(0, 0);
        lcd.print("                ");
        // Tambahkan tindakan default untuk nilai di luar rentang yang diharapkan
    }

    // if (key)
    // {
    // Serial.print("key=");
    // Serial.println(key);
    // Serial.print(" nil.menu=");
    // Serial.println(nilaiMenu);
    // Serial.print(" kec.putar=");
    // Serial.println(kecepatanPutar);
    // Serial.print(" per.putar=");
    // Serial.println(percepatanPutar);
    // Serial.print(" lom.sudut=");
    // Serial.println(lompatanSudut);
    // Serial.print(" lompat=");
    // Serial.println(lompat);
    // Serial.print(" run=");
    // Serial.println(run);
    // lcd.setCursor(14, 0);
    // lcd.print(key);
    // lcd.print("               ");
    // }
}
