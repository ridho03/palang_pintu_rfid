#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Servo.h>

#define SS_PIN1 2   // D4 - Slave Select (SS) untuk modul RFID pertama
#define SS_PIN2 15  // D8 - Slave Select (SS) untuk modul RFID kedua

MFRC522 mfrc522_1(SS_PIN1, 0);  // Inisialisasi modul RFID pertama
MFRC522 mfrc522_2(SS_PIN2, 0);  // Inisialisasi modul RFID kedua

const char* ssid = "wifimubuat";
const char* password = "Marta130420!";
String content;
bool uidExists = false;

Servo servo1;  // Servo pertama
Servo servo2;  // Servo kedua
int servo1Pos = 25;  // Awalnya servo pertama berada pada posisi 45 derajat
int servo2Pos = 25;  // Awalnya servo kedua berada pada posisi 45 derajat

unsigned long previousMillis = 0;
const unsigned long interval = 1000;
Servo activeServo = servo1;  // Inisialisasi servo yang aktif (servo1 atau servo2)

void setup() {
  Serial.begin(115200);
  SPI.begin();

  mfrc522_1.PCD_Init();
  mfrc522_2.PCD_Init();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("Use this URL to connect: http://");
  Serial.print(WiFi.localIP());
  Serial.println("");

  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT); // Konfigurasi pin 4 sebagai output
  servo1.attach(4);  // Pasang servo pertama ke pin 5
  servo2.attach(5);  // Pasang servo kedua ke pin 6
  servo1.write(servo1Pos);  // Setel servo pertama ke posisi awal (45 derajat)
  servo2.write(servo2Pos);  // Setel servo kedua ke posisi awal (45 derajat)
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (WiFi.status() == WL_CONNECTED) {
      uidExists = false; // Setel uidExists ke false setiap kali loop dimulai
      readRFID(mfrc522_1, "Reader 1");
      readRFID(mfrc522_2, "Reader 2");

      // Servo aktif
      if (uidExists) {
        activeServo.write(180);  // Gerakkan servo aktif ke posisi 90 derajat
        delay(5000);  // Tunggu 5 detik
        activeServo.write(25);  // Kembali ke posisi awal (45 derajat)
      }
    } else {
      Serial.println("Error in WiFi connection");
    }
  }
}

void readRFID(MFRC522 mfrc522, String readerName) {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    Serial.print(readerName);
    Serial.print(" UID: ");

    content = "";
    byte letter;

    for (byte i = 0; i < mfrc522.uid.size; i++) {
      Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      Serial.print(mfrc522.uid.uidByte[i], HEX);
      content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      content.concat(String(mfrc522.uid.uidByte[i], HEX));
    }

    content.toUpperCase();
    Serial.println();

    // Tentukan servo aktif berdasarkan modul RFID yang terbaca
    if (readerName == "Reader 1") {
      activeServo = servo1;
    } else if (readerName == "Reader 2") {
      activeServo = servo2;
    }

    sendUIDtoServer(content, readerName);
  }
}

void sendUIDtoServer(String uid, String readerName) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, "http://192.168.244.195/login/dasboard/rest.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String postData = "uid=" + uid + "&readerName=" + readerName; // Kirim parameter readerName
  int httpResponseCode = http.POST(postData);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(response);

    if (response.equals("UID exists")) {
      uidExists = true;
    }
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
}
