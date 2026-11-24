#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

#define SS_PIN 10
#define RST_PIN 9
#define BUZZER_PIN 8
#define LCD_RS 2
#define LCD_EN 3
#define LCD_D4 4
#define LCD_D5 5
#define LCD_D6 6
#define LCD_D7 7
#define ESP_RX 11
#define ESP_TX 12

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
SoftwareSerial espSerial(ESP_RX, ESP_TX);

const String WIFI_SSID = "<WIFI_SSID>";
const String WIFI_PASS = "<WIFI_PASSWORD>";
const String AWS_HOST = "s9i6ja0k92.execute-api.eu-north-1.amazonaws.com";
const String DEVICE_SECRET = "<DEVICE_SECRET>";
const String DEVICE_ID = "GATE-01";

int totalStock = 0;
String lastScannedUID = "";
unsigned long lastScanTimestamp = 0;
const unsigned long DEBOUNCE_DELAY_MS = 3000;
bool wifiReady = false;

void dumpEspResponse(unsigned long timeoutMs = 2000) {
  unsigned long deadline = millis() + timeoutMs;
  while (millis() < deadline) {
    while (espSerial.available()) {
      Serial.write(espSerial.read());
    }
  }
}

void waitForWifi() {
  Serial.println(F("Waiting for Wi-Fi to obtain IP..."));
  unsigned long start = millis();

  while (!wifiReady && (millis() - start) < 20000) {
    static String line = "";
    while (espSerial.available()) {
      char c = espSerial.read();
      Serial.write(c);

      if (c == '\n' || c == '\r') {
        if (line.indexOf("WIFI GOT IP") >= 0) {
          wifiReady = true;
        }
        line = "";
      } else {
        line += c;
      }
    }
  }
}

void setupWiFi() {
  espSerial.println(F("AT+RST"));
  delay(3000);
  dumpEspResponse();

  espSerial.println(F("AT+CWMODE=1"));
  delay(500);
  dumpEspResponse();

  String cmd = "AT+CWJAP=\"" + WIFI_SSID + "\",\"" + WIFI_PASS + "\"";
  espSerial.println(cmd);
  dumpEspResponse(12000);
}

String sendScanToAWS(const String &uid) {
  String path = "/prod/scan?tag=" + uid + "&dev=" + DEVICE_ID +
                "&key=" + DEVICE_SECRET + "&action=IN";

  espSerial.println("AT+CIPSTART=\"SSL\",\"" + AWS_HOST + "\",443");
  delay(1000);
  dumpEspResponse();

  String httpCmd = "GET " + path + " HTTP/1.1\r\nHost: " + AWS_HOST +
                   "\r\nConnection: close\r\n\r\n";

  espSerial.print("AT+CIPSEND=");
  espSerial.println(httpCmd.length());
  delay(300);
  espSerial.print(httpCmd);

  unsigned long timeout = millis() + 4000;
  String result = "";

  while (millis() < timeout) {
    while (espSerial.available()) {
      result += (char)espSerial.read();
    }
    if (result.indexOf('#') >= 0 &&
        result.lastIndexOf('#') > result.indexOf('#')) {
      break;
    }
  }

  return result;
}

void updateLCD(const String &message) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
  lcd.setCursor(0, 1);
  lcd.print("TOTAL STOCK: ");
  lcd.print(totalStock);
}

void successBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

void deniedBuzzer() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(800);
  digitalWrite(BUZZER_PIN, LOW);
}

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  lcd.begin(16, 2);
  lcd.print("WAREHOUSE SYSTEM");
  lcd.setCursor(0, 1);
  lcd.print("TRACKING READY");

  delay(2000);
  updateLCD("SCAN ITEM CARD");
  setupWiFi();
  waitForWifi();
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String currentUID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) {
      currentUID += "0";
    }
    currentUID += String(rfid.uid.uidByte[i], HEX);
  }

  currentUID.toUpperCase();
  unsigned long now = millis();

  if (currentUID == lastScannedUID &&
      (now - lastScanTimestamp) < DEBOUNCE_DELAY_MS) {
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return;
  }

  lastScannedUID = currentUID;
  lastScanTimestamp = now;

  String awsResult = sendScanToAWS(currentUID);

  if (awsResult.indexOf("#IN") >= 0 ||
      awsResult.indexOf("\"status\": \"OK\"") >= 0) {
    totalStock++;
    successBuzzer();
    updateLCD("ITEM ADDED!");
  } else {
    deniedBuzzer();
    updateLCD("UNAUTH TAG!");
  }

  delay(1500);
  updateLCD("SCAN ITEM CARD");
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}
