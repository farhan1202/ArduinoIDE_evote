#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
WiFiClient client;
#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN 2 // pin sda
#define RST_PIN 16 // pin rst
#include "lib/LiquidCrystal_I2C.h"
#include "lib/LiquidCrystal_I2C.cpp"
#include <Wire.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define buzzer 3
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
#include <Adafruit_Fingerprint.h>
#include <SoftwareSerial.h>
SoftwareSerial fpsensor(0, 15);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fpsensor);
String request_string;
const char* host = "192.168.43.166";
int query = 0;
int fingerprintID = 0;
HTTPClient http;
int index1, index2 , index11 , index22;
String no_id, dat2 , no_idd;

void setup() {
  finger.begin(57600);
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  WiFi.disconnect();
  WiFi.begin("bravo2", "11111111");
  while ((!(WiFi.status() == WL_CONNECTED))) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wifi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  }
  else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }
  SPI.begin();
  rfid.PCD_Init();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("    SILAHKAN    ");
  lcd.setCursor(0, 1);
  lcd.print(" MELAKUKAN SCAN ");
  Serial.println("I am waiting for card…");
  pinMode(buzzer, OUTPUT);

}
String strID, val, data, data1;
unsigned long timenow = 0;
boolean state_buzzer = 0; //kondisi
boolean state_kartu = 0; // kondisi

void loop() {
  {
    fingerprintID = getFingerprintIDez();
    delay(50);
    baca_serial1();
  }
  rfid1();


}
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;


  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    alrme();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   SIDIK JARI   ");
    lcd.setCursor(0, 1);
    lcd.print("TIDAK DIKETAHUI");
    Serial.println("not found");

    delay(6000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("    SILAHKAN    ");
    lcd.setCursor(0, 1);
    lcd.print(" MELAKUKAN SCAN ");
    return -1;
  }
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("ID JARI : ");
  lcd.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  query = 1;
  alrm();
  masuk_finger();
  return finger.fingerID;
}
void rfid1()
{
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    return;

  // Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  // Serial.println(rfid.PICC_GetTypeName(piccType));

  //id kartu dan yang akan dikirim ke database
  strID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    strID +=
      (rfid.uid.uidByte[i] < 0x10 ? "0" : "") +
      String(rfid.uid.uidByte[i], HEX) +
      (i != rfid.uid.size - 1 ? ":" : "");
  }

  strID.toUpperCase();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" ID KARTU ANDA ");
  lcd.setCursor(0, 1);
  lcd.print(  strID  );
  Serial.println(strID);
  masuk(strID);
  baca_serial();
  state_buzzer = 0;
  state_kartu = 0;
  alrm();
  delay(200);
}

void masuk_finger()
{
  if (!client.connect(host, 80)) {
    Serial.println("Gagal Konek");
    return;
  }

  request_string = "/pemilu/api/scanFinger.php?idfinger=";
  request_string += finger.fingerID;
  Serial.print("requesting URL: ");
  Serial.println(request_string);
  client.print(String("GET ") + request_string + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  query = 0;
}
void masuk(String id)
{
  if (!client.connect(host, 80)) {
    Serial.println("Gagal Konek");
    return;
  }

  request_string = "/pemilu/api/scanRfid.php?idcard=";
  request_string += id;

  Serial.print("requesting URL: ");
  Serial.println(request_string);
  client.print(String("GET ") + request_string + " HTTP/1.1\r\n" + "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
}
void alrm()
{
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
}
void alrme()
{
  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer, LOW);
  delay(1000);
  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer, LOW);
  delay(1000);
  digitalWrite(buzzer, HIGH);
  delay(1000);
  digitalWrite(buzzer, LOW);
  delay(1000);
}


void baca_serial() {
  while (client.available() > 0) {
    delay(10);
    char c = client.read();
    data += c;
  }
  if (data.length() > 0) {
    Serial.println(data);
    index1 = data.indexOf('%');
    index2 = data.indexOf('%', index1 + 1);
    no_id = data.substring(index1 + 1, index2);
    Serial.print("No ID: ");
    Serial.println(no_id);
    if ( state_buzzer == 0 && no_id == "none") {
      alrme();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("IDENTITAS ANDA ");
      lcd.setCursor(0, 1);
      lcd.print("TIDAK TERDAFTAR! ");
      Serial.println("Identitas Anda Tidak Terdaftar !");
      strID = "";
      state_kartu = 0;
      state_buzzer = 1 ;
      delay(6000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("    SILAHKAN    ");
      lcd.setCursor(0, 1);
      lcd.print(" MELAKUKAN SCAN ");
    }
    else {
      if (state_kartu == 0) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("IDENTITAS ANDA ");
        lcd.setCursor(0, 1);
        lcd.print("    TERDAFTAR!    ");
        Serial.println("Identitas Terdaftar !");
        state_buzzer = 0 ;
        state_kartu = 1;
        delay(6000);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("    SILAHKAN    ");
        lcd.setCursor(0, 1);
        lcd.print(" MELAKUKAN SCAN ");

      }
      data = "";
    }
  }
}
void baca_serial1() {
  while (client.available() > 0) {
    delay(10);
    char c = client.read();
    data1 += c;
  }
  if (data1.length() > 0) {
    Serial.println(data);
    index11 = data1.indexOf('%');
    index22 = data1.indexOf('%', index11 + 1);
    no_idd = data1.substring(index11 + 1, index22);
    Serial.print("No ID: ");
    Serial.println(no_idd);
    if ( no_idd == "none") {
      alrme();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("IDENTITAS ANDA ");
      lcd.setCursor(0, 1);
      lcd.print("TIDAK TERDAFTAR! ");
      Serial.println("Identitas Anda Tidak Terdaftar !");
      delay(100);

      data1 = "";
      delay(6000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("    SILAHKAN    ");
      lcd.setCursor(0, 1);
      lcd.print(" MELAKUKAN SCAN ");

    }
    else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("IDENTITAS ANDA ");
      lcd.setCursor(0, 1);
      lcd.print("    TERDAFTAR!    ");
      Serial.println("Identitas Terdaftar !");
      delay(6000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("    SILAHKAN    ");
      lcd.setCursor(0, 1);
      lcd.print(" MELAKUKAN SCAN ");
      data1 = "";
    }
  }
}


