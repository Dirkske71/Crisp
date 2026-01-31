#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// I2C Adres van je LCD (meestal 0x27)
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Pinnen zoals jij ze hebt aangesloten
#define PIN_INLAAT 4
#define PIN_UITLAAT 13
#define PIN_HEETGAS 14

// Setup voor 3 aparte OneWire bussen
OneWire owInlaat(PIN_INLAAT);
OneWire owUitlaat(PIN_UITLAAT);
OneWire owHeetgas(PIN_HEETGAS);

DallasTemperature sensorInlaat(&owInlaat);
DallasTemperature sensorUitlaat(&owUitlaat);
DallasTemperature sensorHeetgas(&owHeetgas);

void setup() {
  // Start I2C op de standaard pinnen (21 SDA, 22 SCL)
  Wire.begin(21, 22);
  
  lcd.init();
  lcd.backlight();
  
  // Start de sensoren
  sensorInlaat.begin();
  sensorUitlaat.begin();
  sensorHeetgas.begin();

  lcd.setCursor(0, 0);
  lcd.print("WILLY PRO SENSOR TEST");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Vraag temperaturen op
  sensorInlaat.requestTemperatures();
  sensorUitlaat.requestTemperatures();
  sensorHeetgas.requestTemperatures();

  // Lees waarden
  float tIn = sensorInlaat.getTempCByIndex(0);
  float tUit = sensorUitlaat.getTempCByIndex(0);
  float tGas = sensorHeetgas.getTempCByIndex(0);

  // Weergave op LCD
  printTemp("Inlaat : ", tIn, 0);
  printTemp("Uitlaat: ", tUit, 1);
  printTemp("Heetgas: ", tGas, 2);

  lcd.setCursor(0, 3);
  lcd.print("Systeem: OK         ");

  delay(1000); // Update elke seconde
}

// Hulpfunctie om netjes te printen
void printTemp(String label, float temp, int row) {
  lcd.setCursor(0, row);
  lcd.print(label);
  
  if (temp == DEVICE_DISCONNECTED_C) {
    lcd.print("FOUT!   ");
  } else {
    lcd.print(temp, 1);
    lcd.print((char)223); // Graden symbool
    lcd.print("C   ");
  }
}