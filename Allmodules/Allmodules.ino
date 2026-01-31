 #include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

const int pins[] = {4, 13, 14, 32, 33, 26, 12}; 
const char* labels[] = {"TS", "TL", "TP", "TC", "TA", "CI", "CO"};

OneWire ow[] = { OneWire(pins[0]), OneWire(pins[1]), OneWire(pins[2]), OneWire(pins[3]), OneWire(pins[4]), OneWire(pins[5]), OneWire(pins[6]) };
DallasTemperature sensors[7] = { &ow[0], &ow[1], &ow[2], &ow[3], &ow[4], &ow[5], &ow[6] };

void setup() {
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  
  for(int i=0; i<7; i++) {
    sensors[i].begin();
    sensors[i].setWaitForConversion(false); 
  }
  
  lcd.clear();
  
  // STATISCHE LABELS (Eén keer printen voor rustig beeld)
  lcd.setCursor(0, 0); lcd.print("TS:      TL:");
  lcd.setCursor(0, 1); lcd.print("TP:      TC:");
  lcd.setCursor(0, 2); lcd.print("TA:      CI:");
  lcd.setCursor(0, 3); lcd.print("CO:      |WILCO PRO "); // Exact 20 tekens
}

void loop() {
  for(int i=0; i<7; i++) {
    sensors[i].requestTemperatures();
  }

  // Update de waarden op de gereserveerde plekken
  updateValue(3, 0, 0);  // TS
  updateValue(13, 0, 1); // TL
  updateValue(3, 1, 2);  // TP
  updateValue(13, 1, 3); // TC
  updateValue(3, 2, 4);  // TA
  updateValue(13, 2, 5); // CI
  updateValue(3, 3, 6);  // CO

  delay(400); 
}

void updateValue(int x, int y, int index) {
  lcd.setCursor(x, y);
  float t = sensors[index].getTempCByIndex(0);
  
  if (t == DEVICE_DISCONNECTED_C || t == 85.00) {
    lcd.print("F!   "); // De enige echte Willy-foutmelding
  } else {
    // Zorgt dat getallen netjes op hun plek blijven staan
    if (t < 10.0 && t >= 0) lcd.print(" "); 
    if (t < 100.0 && t >= 10.0) ; // Normale positie
    
    lcd.print(t, 1);
    lcd.print(" "); // Overschrijft eventuele restanten
  }
}