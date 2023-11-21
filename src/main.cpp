#include <Arduino.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 20, 4); // set lcd

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

const byte ROWS = 2; // four rows
const byte COLS = 4; // three columns
char keys[ROWS][COLS] = {
    {'1', '2', '3', '4'},
    {'A', 'B', 'C', 'D'}};
byte rowPins[ROWS] = {6, 7};       // connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4, 5}; // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// semua variable disini adalah variable default untuk flash firmware apa awal isi software
// semua variabel tentang stepper motor
const int stepPin = 11;
unsigned long stepPinInterval; // interval atau lama step pin mati/nyala
const float dirPin = 12;
bool hold = false;

// variabel2 yang penting untuk tampilan layar dan letak variabel
int nilaiMenu = 0;
bool doneSend = true; // default adalah true agar tidak ada yang dikirim ketika awal program berjalan

// variabel penting / utama
int feedrate = 10;    // persen
int akselerasi = 10;  // derajat per detik kuadrat */s^2
bool arah = true;     // true artinya cw dan false artinya ccw
bool run = false;     // true artinya run, dan false artinya stop
int derajatBenda = 0; // nilai derajat.
int nilaiLompat = 10; // setiap melompat sejauh 45 derajat (ganti ke eeprom nanti)
bool lompat = false;
int penambahanNilai = 0;

void setup()
{
  Serial.begin(115200);
  lcd.init(); // initialize the lcd
  // Print a message to the LCD
  lcd.backlight();
  lcd.createChar(0, ikonDerajat); // ikon home
  lcd.createChar(3, ikonSekat);   // ikon sekat
  lcd.createChar(4, ikonLompat);  // ikon lompat

  EEPROM.get(0, hold);         // eeprom untuk hold
  EEPROM.get(10, feedrate);    // feedrate
  EEPROM.get(20, akselerasi);  // akselerasi
  EEPROM.get(30, nilaiLompat); // nilai lompat
}

void loop()
{
  // semua program tentang fungsi tombol di tampilan utama ada disini
  char key = keypad.getKey();

  if (key == '1') // tombol menu ditekan. pergi ke menu
  {
    nilaiMenu++;
    if (nilaiMenu > 5)
      nilaiMenu = 1;
  }

  if (key == 'D') // tombol menu ditekan. pergi ke menu
  {
    penambahanNilai++;
    if (penambahanNilai > 5)
      penambahanNilai = 0;
  }

  if (key == '4')
  {
    nilaiMenu = 0;
    if (hold == true)
      Serial.println("$1=255");
    else
      Serial.println("$1=0");
    delay(300);
    Serial.print("$120=");
    Serial.println(akselerasi);
    EEPROM.put(0, hold);         // eeprom untuk hold
    EEPROM.put(10, feedrate);    // feedrate
    EEPROM.put(20, akselerasi);  // akselerasi
    EEPROM.put(30, nilaiLompat); // nilai lompat
  }

  if (key == 'A') // tombol arah ditekan. ganti cw atau ccw
  {
    arah = !arah;
  }

  if (key == 'B') // jika tombol run ditekan, jalankan atau hentikan motor
  {
    run = !run;
    doneSend = false;
  }

  if (key == 'C') // tombol lompat ditekan, lompat sekarang
  {
    lompat = true;
    Serial.print("$J=G21G91X");
    if (arah == false)
      Serial.print("-");
    Serial.print(nilaiLompat);
    Serial.print("Y0.000Z0.000F");
    Serial.println(feedrate);
    doneSend = true;
  }

  // program semua tentang stepper
  if (doneSend == false)
  {
    if (run == false)
    {
      Serial.println("!");
      doneSend = true;
    }
    else if (run == true)
    {
      Serial.print("$J=G21G91X");
      if (arah == false)
        Serial.print("-");
      Serial.print("400.00Y0.000Z0.000F");
      Serial.println(feedrate);
      doneSend = true;
    }
  }

  // jika menu > 1 maka tampilkan beberapa variabel yang dapat diubah
  //  jika menu == 0 maka tampilkan menu utama
  // Gunakan switch case untuk menentukan aksi berdasarkan nilai sensor
  switch (nilaiMenu)
  {
  case 0:
    lcd.setCursor(0, 0);
    lcd.print(feedrate); // satuan mm/min
    lcd.print("%");
    lcd.write(byte(3));    // sekat
    lcd.print(akselerasi); // satuan mm/min^2
    lcd.write(byte(3));    // sekat
    lcd.print("x");
    lcd.print(nilaiLompat);
    lcd.write(byte(0)); // derajat
    lcd.write(byte(3)); // sekat
    if (hold == true)   // mode relase atau hold ada disini
      lcd.print("H");
    else
      lcd.print("R");
    lcd.print("               ");
    // proogram menampilkan ui baris kedua
    lcd.setCursor(0, 1);
    if (arah == true)
      lcd.print("CW");
    else
      lcd.print("CCW");
    lcd.write(byte(3)); // sekat
    if (run == true)
      lcd.print("RUN");
    else
      lcd.print("STOP");
    lcd.write(byte(3)); // sekat
    lcd.write(byte(4));
    lcd.print("               ");
    break;
  case 1:
    lcd.setCursor(0, 0);
    lcd.print("Feedrate         ");
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.print(feedrate);
    lcd.print(" (x10^");
    lcd.print(penambahanNilai);
    lcd.print(")        ");
    if (key == '2')
    {
      feedrate = feedrate + pow(10, penambahanNilai);
    }
    if (key == '3')
      feedrate = feedrate - pow(10, penambahanNilai);
    break;
  case 2:
    lcd.setCursor(0, 0);
    lcd.print("Akselerasi   ");
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.print(akselerasi);
    lcd.print(" (x10^");
    lcd.print(penambahanNilai);
    lcd.print(")        ");
    lcd.print("                ");
    if (key == '2')
      akselerasi += pow(10, penambahanNilai);
    if (key == '3')
      akselerasi -= pow(10, penambahanNilai);
    break;
  case 3:
    lcd.setCursor(0, 0);
    lcd.print("Lompatan Sudut  ");
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.print(nilaiLompat);
    lcd.print(" (x10^");
    lcd.print(penambahanNilai);
    lcd.print(")        ");
    lcd.print("                ");
    if (key == '2')
      nilaiLompat = nilaiLompat + pow(10, penambahanNilai);
    if (key == '3')
      nilaiLompat = nilaiLompat - pow(10, penambahanNilai);
    break;
  case 4: // mode hold atau relase
    lcd.setCursor(0, 0);
    lcd.print("Mode Hold?:     ");
    lcd.setCursor(0, 1);
    lcd.print(" ");
    if (hold)
      lcd.print("Hold (H)");
    else
      lcd.print("Relase (R)");
    lcd.print(hold);
    lcd.print("                ");
    if (key == '2')
      hold = false;
    if (key == '3')
      hold = true;
    break;
  case 5: // mode hold atau relase
    lcd.setCursor(0, 0);
    lcd.print("Reset EEPROM:     ");
    lcd.setCursor(0, 1);
    lcd.print(" Tekan 3 u/ ok    ");
    if (key == '3')
    {
      lcd.setCursor(0, 1);
      lcd.print(" Melakukan Reset    ");
      for (unsigned int i = 0; i < EEPROM.length(); i++) // direset,
        EEPROM.write(i, 0);
      delay(100);
      lcd.setCursor(0, 1);
      lcd.print(" Selesai Reset     ");
      // dapatkan nilai dari eeprom setelah diupdate, atau di reset
      // lalu update dan tampilkan hasil yang telah jadi
      EEPROM.get(0, hold);         // eeprom untuk hold
      EEPROM.get(10, feedrate);    // feedrate
      EEPROM.get(20, akselerasi);  // akselerasi
      EEPROM.get(30, nilaiLompat); // nilai lompat
    }
    break;
  default:
    // proogram menampilkan ui baris pertama
    lcd.setCursor(0, 0);
    lcd.print("ERROR!!?!       ");
    lcd.setCursor(0, 0);
    lcd.print("                ");
  }
}
