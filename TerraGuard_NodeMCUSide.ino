#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <addons/RTDBHelper.h>
#define WIFI_SSID "Midnight"
#define WIFI_PASSWORD "meetMe@Mid2023"


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
TinyGPSPlus gps;


SoftwareSerial espSerial(1, 0); 
SoftwareSerial SerialGPS(4, 5);

// GPS data
float Latitude, Longitude;
int year, month, date, hour, minute;
String DateString, TimeString, LatitudeString, LongitudeString;

void readGPSData() {

  while (SerialGPS.available() > 0) {
    if (gps.encode(SerialGPS.read())) {
      if (gps.location.isValid()) {
        Latitude = gps.location.lat();
        Longitude = gps.location.lng();
        LatitudeString = String(Latitude, 6);
        LongitudeString = String(Longitude, 6);
      }

      if (gps.date.isValid()) {
        DateString = "";
        date = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();
      }

      if (gps.time.isValid()) {
        TimeString = "";
        hour = gps.time.hour() + 8;
        minute = gps.time.minute();
      }
    }
  }
}

void connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  // Check if Wi-Fi connection is successful
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting");
    // Reconnect every 10 seconds
    while (true) {
      delay(10000);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to Wi-Fi");
        break;
      }
    }
  } else {
    Serial.println("Connected to Wi-Fi");
  }
}
void setup() {
  Serial1.begin(9600);
  Serial.begin(9600);
  SerialGPS.begin(9600);

  connectToWifi();

  config.database_url = "https://temphum-4ede4-default-rtdb.firebaseio.com";
  config.signer.tokens.legacy_token = "vouL4do8gj8Y1omI31S5TFZ4wrSKdkW1ZDWV5m1h";


  fbdo.setBSSLBufferSize(4096, 1024);

  Firebase.begin(&config, &auth);
}

void loop() {

  if (Serial.available()) {
    // Read data from Arduino Uno
    String data = Serial.readStringUntil('\n');
    Serial.println(data);
    Serial.println(LatitudeString);
    Serial.println(LongitudeString);
    Serial.println(DateString);
    Serial.println(TimeString);
   
    // Set the data in the JSON object
    FirebaseJson firebaseJson;
    firebaseJson.set("n", data.substring(data.indexOf("n:") + 2, data.indexOf(", p:")));
    firebaseJson.set("p", data.substring(data.indexOf("p:") + 2, data.indexOf(", k:")));
    firebaseJson.set("k", data.substring(data.indexOf("k:") + 2));
    firebaseJson.set("lat", LatitudeString);
    firebaseJson.set("long", LongitudeString);
    firebaseJson.set("date", DateString);
    firebaseJson.set("time", TimeString);

    // Send data to Firebase
    Firebase.RTDB.pushJSON(&fbdo, "/NPK", &firebaseJson);
  }
}
