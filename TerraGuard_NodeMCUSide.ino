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
SoftwareSerial SerialGPS(D2, D1); // Using D2 and D1 for GPS
String WifiStat = "";
String FirebStat = "No Data";
String GpsStat = "No Location";
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
// Response
String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
s += "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
s += "<title>TerraGuard Server</title><style>";
s += "body, h1, h2, p, a, footer { margin: 0; padding: 0; font-family: 'Arial', sans-serif; }";
s += "body { background-color: white; color: #046307; display: flex; flex-direction: column; align-items: center; justify-content: flex-start; min-height: 100vh; padding: 20px; }";
s += ".dashboard-header { text-align: center; margin: 20px 0; }";
s += ".cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; width: 100%; max-width: 1200px; }";
s += ".card { background-color: #e8f5e9; border: 1px solid #046307; border-radius: 8px; padding: 20px; box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1); text-align: center; }";
s += ".card h2 { margin-bottom: 15px; }";
s += ".card p { font-size: 1.25rem; margin-bottom: 5px; }";
s += "a { color: #046307; text-decoration: none; font-size: 1rem; }";
s += "footer { text-align: center; margin-top: auto; padding: 20px 0; width: 100%; background-color: #f0f0f0; }";
s += "</style></head><body>";

s += "<div class=\"dashboard-header\"><h1>TerraGuard Quick Dashboard</h1></div>";

s += "<div class=\"cards\">";

s += "<div class=\"card\"><h2>WiFi Status</h2><p>" + WifiStat + "</p></div>";
s += "<div class=\"card\"><h2>Data Status</h2><p>" + FirebStat + "</p></div>";
s += "<div class=\"card\"><h2>Location Status</h2><p>" + GpsStat + "</p></div>";
s += "<hr>";
s += "<div class=\"card\"><h2>Latitude</h2><p>" + LatitudeString + "</p></div>";
s += "<div class=\"card\"><h2>Longitude</h2><p>" + LongitudeString + "</p></div>";
s += "<div class=\"card\"><h2>Date</h2><p>" + DateString + "</p></div>";
s += "<div class=\"card\"><h2>Time</h2><p>" + TimeString + "</p></div>";

s += "</div>";

// Include the GPS map link if the GPS location is valid
if (gps.location.isValid()) {
    s += "<div class=\"dashboard-header\">";
    s += "<p><a href=\"http://maps.google.com/maps?&z=15&mrt=yp&t=k&q=" + LatitudeString + "+" + LongitudeString + "\" target=\"_top\">Click here</a> to open the location in Google Maps.</p>";
    s += "</div>";
}

s += "<footer><p>Powered by <b><i>DimenSoft<i><b></p></footer>";

s += "</body></html>";

client.print(s);
  delay(100);
}
