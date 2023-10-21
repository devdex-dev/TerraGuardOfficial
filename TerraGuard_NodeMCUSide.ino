#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <FirebaseESP8266.h>

// Initialize GPS and Serial
TinyGPSPlus gps;
SoftwareSerial SerialGPS(4, 5);

// Firebase objects
FirebaseData firebaseData;
FirebaseJson firebaseJson;

// GPS data
  float Latitude, Longitude;
  int year, month, date, hour, minute;
  String DateString, TimeString, LatitudeString, LongitudeString;

WiFiServer server(80);
// WiFi configuration
const char* ssid = "Midnight";
const char* password = "meetMe@Mid2023";

// Firebase configuration




void setupWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
}

void setupFirebase() {
  FirebaseConfig config;
  config.host = "https://temphum-4ede4-default-rtdb.firebaseio.com/";
  config.api_key = "ZC8aQ7uHOT6n3AgQp2JQyDwClkK9nwXt1ddQ8v0a";
  Firebase.begin(&config, NULL);
}

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

void sendHTMLResponse(WiFiClient& client) {

  String html = "<!DOCTYPE html><html><head><title>NEO-6M GPS Readings</title>";
  html += "<style>table, th, td {border: 1px solid blue;}</style></head><body>";
  html += "<h1 style='font-size:300%;' ALIGN='CENTER'>NEO-6M GPS Readings</h1>";
  html += "<p ALIGN='CENTER' style='font-size:150%;'><b>Location Details</b></p>";
  html += "<table ALIGN='CENTER' style='width:50%'>";
  html += "<tr><th>Latitude</th><td ALIGN='CENTER'>" + LatitudeString + "</td></tr>";
  html += "<tr><th>Longitude</th><td ALIGN='CENTER'>" + LongitudeString + "</td></tr>";
  html += "<tr><th>Date</th><td ALIGN='CENTER'>" + DateString + "</td></tr>";
  html += "<tr><th>Time</th><td ALIGN='CENTER'>" + TimeString + "</td></tr>";
  html += "</table>";

  if (gps.location.isValid()) {
    String mapLink = "http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=" + LatitudeString + "+" + LongitudeString;
    html += "<p align='center'><a style='color:RED; font-size:125%;' href='" + mapLink + "' target='_top'>Click here to open the location in Google Maps.</a></p>";
  }

  html += "</body></html>\n";
  client.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + html);
}

void sendSensorDataToFirebase(String data) {
  firebaseJson.clear();
  firebaseJson.set("n", data.substring(data.indexOf("n:") + 2, data.indexOf(", p:")));
  firebaseJson.set("p", data.substring(data.indexOf("p:") + 2, data.indexOf(", k:")));
  firebaseJson.set("k", data.substring(data.indexOf("k:") + 2));
  firebaseJson.set("lat", LatitudeString);
  firebaseJson.set("long", LongitudeString);
  firebaseJson.set("date", DateString);
  firebaseJson.set("time", TimeString);
  Firebase.pushJSON(firebaseData, "/NPK", firebaseJson);
}

void setup() {
  Serial.begin(9600);
  SerialGPS.begin(9600);

  setupWiFi();
  setupFirebase();
}

void loop() {
  readGPSData();

  WiFiClient client = server.available();
  if (client) {
    sendHTMLResponse(client);
    delay(100);
  }

  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    Serial.println(data);
    sendSensorDataToFirebase(data);
  }
}
