#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// I2C Adres van je LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

// De 7 pinnen voor je sensoren
const int pins[] = {4, 13, 14, 32, 33, 26, 12}; 
const char* labels[] = {"TS", "TL", "TP", "TC", "TA", "CI", "CO"};

// Setup arrays voor de OneWire bussen en Dallas sensoren
OneWire ow[] = { OneWire(pins[0]), OneWire(pins[1]), OneWire(pins[2]), OneWire(pins[3]), OneWire(pins[4]), OneWire(pins[5]), OneWire(pins[6]) };
DallasTemperature sensors[7] = { &ow[0], &ow[1], &ow[2], &ow[3], &ow[4], &ow[5], &ow[6] };

void setup() {
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  
  lcd.setCursor(0, 0);
  lcd.print("WILLY PRO SENSOR TEST");
  
  for(int i=0; i<7; i++) {
    sensors[i].begin();
    sensors[i].setWaitForConversion(false); // Snelheid optimalisatie
  }
  
  delay(2000);
  lcd.clear();
  
  // STATISCHE LABELS (Eén keer printen voor rustig beeld)
  lcd.setCursor(0, 0); lcd.print("TS:      TL:");
  lcd.setCursor(0, 1); lcd.print("TP:      TC:");
  lcd.setCursor(0, 2); lcd.print("TA:      CI:");
  lcd.setCursor(0, 3); lcd.print("CO:      |WILCO PRO ");
}

void loop() {
  // Vraag alle temperaturen op
  for(int i=0; i<7; i++) {
    sensors[i].requestTemperatures();
  }

  // Update de waarden op de gereserveerde plekken
  updateValue(3, 0, 0);  // TS (kolom 3, rij 0, sensor 0)
  updateValue(13, 0, 1); // TL (kolom 13, rij 0, sensor 1)
  updateValue(3, 1, 2);  // TP
  updateValue(13, 1, 3); // TC
  updateValue(3, 2, 4);  // TA
  updateValue(13, 2, 5); // CI
  updateValue(3, 3, 6);  // CO

  delay(1000); 
}

// Functie om alleen de waarde te verversen
void updateValue(int x, int y, int index) {
  lcd.setCursor(x, y);
  float t = sensors[index].getTempCByIndex(0);
  
  if (t == DEVICE_DISCONNECTED_C || t == 85.00) {
    lcd.print("F!   "); 
  } else {
    // Zorgt voor nette uitlijning (ruimte voor 1 decimaal)
    if (t < 10.0 && t >= 0) lcd.print(" "); 
    lcd.print(t, 1);
    lcd.print(" "); // Overschrijft eventuele restanten
  }
}