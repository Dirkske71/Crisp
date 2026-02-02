#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// I2C Adres van je LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);

// JOUW MAC-ADRES ONTVANGER (Steam OLED)
uint8_t broadcastAddress[] = {0xB0, 0xCB, 0xD8, 0xDA, 0xF5, 0x48}; 

// Configuratie van de sensoren
const int numSensors = 7;
const int pins[] = {4, 13, 14, 32, 33, 26, 12}; 
const char* labels[] = {"TS", "TL", "TP", "TC", "TA", "CI", "CO"};

OneWire ow[] = { OneWire(pins[0]), OneWire(pins[1]), OneWire(pins[2]), OneWire(pins[3]), OneWire(pins[4]), OneWire(pins[5]), OneWire(pins[6]) };
DallasTemperature sensors[numSensors] = { &ow[0], &ow[1], &ow[2], &ow[3], &ow[4], &ow[5], &ow[6] };

typedef struct struct_message {
  float temps[numSensors];
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

void setup() {
  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();

  // ESP-NOW Setup
  WiFi.mode(WIFI_STA);
  esp_now_init();
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Sensoren Setup
  for(int i=0; i<numSensors; i++) {
    sensors[i].begin();
    sensors[i].setWaitForConversion(false);
  }

  lcd.clear();
  // STATISCHE LABELS (Eén keer printen voor rustig beeld)
  lcd.setCursor(0, 0); lcd.print("TS:      TL:");
  lcd.setCursor(0, 1); lcd.print("TP:      TC:");
  lcd.setCursor(0, 2); lcd.print("TA:      CI:");
  lcd.setCursor(0, 3); lcd.print("CO:      |WIFI OK ");
}

void loop() {
  // 1. Sensoren uitlezen & Data inpakken
  for(int i=0; i<numSensors; i++) {
    sensors[i].requestTemperatures();
    myData.temps[i] = sensors[i].getTempCByIndex(0);
  }

  // 2. LOKALE WEERGAVE (Update alleen de getallen)
  updateLcdValue(3, 0, 0);  // TS
  updateLcdValue(13, 0, 1); // TL
  updateLcdValue(3, 1, 2);  // TP
  updateLcdValue(13, 1, 3); // TC
  updateLcdValue(3, 2, 4);  // TA
  updateLcdValue(13, 2, 5); // CI
  updateLcdValue(3, 3, 6);  // CO

  // 3. DRAADLOOS VERZENDEN naar Steam OLED
  esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

  delay(1000); 
}

// Functie voor nette lokale weergave
void updateLcdValue(int x, int y, int index) {
  lcd.setCursor(x, y);
  float t = myData.temps[index];
  
  if (t == DEVICE_DISCONNECTED_C || t == 85.00) {
    lcd.print("F!   "); 
  } else {
    if (t < 10.0 && t >= 0) lcd.print(" "); 
    lcd.print(t, 1);
    lcd.print(" "); 
  }
}
