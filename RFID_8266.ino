#include <SPI.h>
#include "MFRC522.h"
#include <FirebaseESP8266.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <TimeLib.h>

#define RST_PIN D1
#define SS_PIN D2
#define WIFI_SSID "Solomon" 
#define WIFI_PASSWORD "0878872337bo"
#define FIREBASE_HOST "rfid-database-8c0f2-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_KEY "2nyECFKpBSthELM3cbV6rjxEQodeJSuaTQviU2iF"
#define RELAY_PIN D4

FirebaseData firebaseData;
MFRC522 mfrc522(SS_PIN, RST_PIN);
String rfid_in = "";

void setup() {
  Serial.begin(9600);
  connectWifi();
  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("");
  Firebase.begin(FIREBASE_HOST, FIREBASE_KEY);
  setSyncProvider(getNtpTime);
  pinMode(RELAY_PIN, OUTPUT);
  while (timeStatus() == timeNotSet) {
    delay(1000);
    Serial.println("Waiting for time synchronization...");
  }
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    rfid_in = rfid_read();
    Serial.println("===> " + rfid_in);
    sendToFirebase();
    digitalWrite(RELAY_PIN, HIGH);
    delay(1000);
    digitalWrite(RELAY_PIN, LOW);
  }
  delay(1);
}

String rfid_read() {
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();
  return content.substring(1);
}

//ConnectWIFI

void connectWifi() {
    Serial.begin(9600);
    Serial.println(WiFi.localIP());
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("connecting");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
}
//SaveonFirebase
void sendToFirebase() {
  delay(1000);

  // สร้างโครงสร้างข้อมูลที่คุณต้องการบันทึกลง Firebase
  // ในที่นี้คือ Key, ชื่อ, นามสกุล และเวลา
  String currentDate = getCurrentDate();
  String checkinTime = getCurrentTime();
  String checkoutTime = getCurrentTime();
  
  String path = "/RFID/Datetime/" + currentDate + "/" + rfid_in;
  FirebaseJson json;

  if (Firebase.getString(firebaseData, path + "/Checkin")) {
    // ถ้ามีข้อมูลใน "Checkin" แสดงว่ามีการแตะบัตรครั้งแรกแล้ว
    // เราจะอัปเดต "Checkout" และบันทึกเวลาใน "Checkout" child
    path += "/Checkout";
    json.add("Time", checkoutTime);
  } else {
    // ถ้ายังไม่มีข้อมูลใน "Checkin" แสดงว่านี่คือการแตะบัตรครั้งแรก
    // เราจะอัปเดต "Checkin" และบันทึกเวลาใน "Checkin" child
    path += "/Checkin";
    json.add("Time", checkinTime);
  }

  if (Firebase.setJSON(firebaseData, path, json)) {
    Serial.println("Added/Updated");
  } else {
    Serial.println("Error: " + firebaseData.errorReason());
  }
}

//Datetime
time_t getNtpTime() {
  // เชื่อมต่อกับ NTP Server ของไทยและรับเวลาปัจจุบัน
  configTime(7 * 3600, 0, "th.pool.ntp.org", "time.nectec.or.th"); // ปรับเวลาโซน 7 ชั่วโมง (GMT+7)

  Serial.println("Fetching NTP time from Thai servers...");
  time_t now = time(nullptr);
  return now;
}

String getCurrentDate() {
  // รับเวลาปัจจุบัน
  time_t current_time = now();

  // สร้าง String เพื่อเก็บค่าเวลาในรูปแบบที่ต้องการ
  String Date = "";
  Date += day(current_time);
  Date += "-";
  Date += month(current_time);
  Date += "-";
  Date += year(current_time);
  // Date += " ";
  // dateTime += hour(current_time)+7;
  // dateTime += ":";
  // dateTime += minute(current_time);
  // dateTime += ":";
  // dateTime += second(current_time);

  return Date;
}
String getCurrentTime() {
  // รับเวลาปัจจุบัน
  time_t current_time = now();

  // สร้าง String เพื่อเก็บค่าเวลาในรูปแบบที่ต้องการ
  String Time = "";
  // dateTime += day(current_time);
  // dateTime += "-";
  // dateTime += month(current_time);
  // dateTime += "-";
  // dateTime += year(current_time);
  // dateTime += " ";
  Time += hour(current_time)+7;
  Time += ":";
  Time += minute(current_time);
  Time += ":";
  Time += second(current_time);

  return Time;
}