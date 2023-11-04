#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <addons/RTDBHelper.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define WIFI_SSID "Midnight"
#define WIFI_PASSWORD "meetMe@Mid2023"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
TinyGPSPlus gps;
SoftwareSerial SerialGPS(0, 2);
String WifiStat = "";
String FirebStat = "No Data";
String GpsStat = "";
float Latitude, Longitude;
int year, month, date, hour, minute, second;
String DateString, TimeString, LatitudeString, LongitudeString;
WiFiServer server(80);

void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  // Clear the buffer.
  display.clearDisplay();
  display.display();
  //TG NAME
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0, 24);
  display.setTextSize(2);
  display.print("TerraGuard");
  display.display();
  delay(5000);
  display.clearDisplay();

  //Wifi
  wifiCon();
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());

  // Firebase Config
  config.database_url = "https://temphum-4ede4-default-rtdb.firebaseio.com";
  config.signer.tokens.legacy_token = "vouL4do8gj8Y1omI31S5TFZ4wrSKdkW1ZDWV5m1h";

  fbdo.setBSSLBufferSize(4096, 1024);
  Firebase.begin(&config, &auth);
  // Firebase COnfig
}

void loop() {
  // 
  ArduinoData();
}


void updateDisplay() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("WiFi:");
  display.print(WifiStat);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 12);
  display.print("Data:");
  display.print(FirebStat);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 24);
  display.print("Location:");
  display.print(GpsStat);

  display.display();
}
void wifiCon() {
  //WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Check if Wi-Fi connection is successful
  if (WiFi.status() != WL_CONNECTED) {
    WifiStat = "Connecting";
    updateDisplay();
    Serial.println(WifiStat);
    // Reconnect every 10 seconds
    while (true) {
      delay(10000);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

      if (WiFi.status() == WL_CONNECTED) {
        WifiStat = "Connected";
        updateDisplay();
        Serial.println(WifiStat);
        break;
      }
    }

  } else {
    WifiStat = "Connected";
    updateDisplay();
  }
  //WIFI
}
void ArduinoData() {
  GetGps();
  if (Serial.available()) {
    String data = Serial.readStringUntil('\n');
    Serial.println(data);
    FirebaseJson firebaseJson;
    firebaseJson.set("n", data.substring(data.indexOf("n:") + 2, data.indexOf(", p:")));
    firebaseJson.set("p", data.substring(data.indexOf("p:") + 2, data.indexOf(", k:")));
    firebaseJson.set("k", data.substring(data.indexOf("k:") + 2));
    firebaseJson.set("lat", LatitudeString);
    firebaseJson.set("long", LongitudeString);
    firebaseJson.set("date", DateString);
    firebaseJson.set("time", TimeString);
    FirebStat = "Uploading";
    updateDisplay();

    if (Firebase.RTDB.pushJSON(&fbdo, "/NPK", &firebaseJson)) {
      FirebStat = "Success";
      updateDisplay();
      Serial.println(FirebStat);
      delay(5000);
      FirebStat = "No Data";
      updateDisplay();

    } else {
      FirebStat = "Fail";
      updateDisplay();
      // Serial.println(FirebStat);
    }
  }
}

void GetGps() {
  while (SerialGPS.available() > 0)
    if (gps.encode(SerialGPS.read())) {
      if (gps.location.isValid()) {
        Latitude = gps.location.lat();
        LatitudeString = String(Latitude, 6);
        Longitude = gps.location.lng();
        LongitudeString = String(Longitude, 6);
      } else {
        // Handle case when GPS location is not valid
        Serial.println("GPS location not valid.");
        // You can set LatitudeString and LongitudeString to an appropriate error message here
        GpsStat = "Location Not Found";
        updateDisplay();
      }

      if (gps.date.isValid()) {
        DateString = "";
        date = gps.date.day();
        month = gps.date.month();
        year = gps.date.year();

        if (date < 10)
          DateString = '0';
        DateString += String(date);

        DateString += " / ";

        if (month < 10)
          DateString += '0';
        DateString += String(month);
        DateString += " / ";

        if (year < 10)
          DateString += '0';
        DateString += String(year);
      }

      if (gps.time.isValid()) {
        TimeString = "";
        hour = gps.time.hour() + 5;  //adjust UTC
        minute = gps.time.minute();
        second = gps.time.second();

        if (hour < 10)
          TimeString = '0';
        TimeString += String(hour);
        TimeString += " : ";

        if (minute < 10)
          TimeString += '0';
        TimeString += String(minute);
        TimeString += " : ";

        if (second < 10)
          TimeString += '0';
        TimeString += String(second);
      }
    }
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  //Response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n <!DOCTYPE html> <html> <head> <title>NEO-6M GPS Readings</title> <style>";
  s += "table, th, td {border: 1px solid blue;} </style> </head> <body> <h1  style=";
  s += "font-size:300%;";
  s += " ALIGN=CENTER>NEO-6M GPS Readings</h1>";
  s += "<p ALIGN=CENTER style="
       "font-size:150%;"
       "";
  s += "> <b>Location Details</b></p> <table ALIGN=CENTER style=";
  s += "width:50%";
  s += "> <tr> <th>Latitude</th>";
  s += "<td ALIGN=CENTER >";
  s += LatitudeString;
  s += "</td> </tr> <tr> <th>Longitude</th> <td ALIGN=CENTER >";
  s += LongitudeString;
  s += "</td> </tr> <tr>  <th>Date</th> <td ALIGN=CENTER >";
  s += DateString;
  s += "</td></tr> <tr> <th>Time</th> <td ALIGN=CENTER >";
  s += TimeString;
  s += "</td>  </tr> </table> ";


  if (gps.location.isValid()) {
    s += "<p align=center><a style="
         "color:RED;font-size:125%;"
         " href="
         "http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=";
    s += LatitudeString;
    s += "+";
    s += LongitudeString;
    s += ""
         " target="
         "_top"
         ">Click here</a> to open the location in Google Maps.</p>";
  }

  s += "</body> </html> \n";

  client.print(s);
  delay(100);
}
