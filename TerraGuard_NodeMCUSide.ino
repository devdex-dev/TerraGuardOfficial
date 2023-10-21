#include <TinyGPS++.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <FirebaseESP8266.h>

TinyGPSPlus gps;
SoftwareSerial SerialGPS(4, 5);
// WiFi configuration
const char* ssid = "Midnight";
const char* password = "meetMe@Mid2023";

float Latitude, Longitude;
int year, month, date, hour, minute;
String DateString, TimeString, LatitudeString, LongitudeString;

WiFiServer server(80);

// Firebase configuration
// #define FIREBASE_HOST "https://temphum-4ede4-default-rtdb.firebaseio.com/"
// #define FIREBASE_AUTH "ZC8aQ7uHOT6n3AgQp2JQyDwClkK9nwXt1ddQ8v0a"

// Firebase object
FirebaseData firebaseData;
FirebaseJson firebaseJson;
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
void setup() {
  config.host =  "https://temphum-4ede4-default-rtdb.firebaseio.com/";
  config.api_key =  "ZC8aQ7uHOT6n3AgQp2JQyDwClkK9nwXt1ddQ8v0a";
  Serial.begin(9600);
  SerialGPS.begin(9600);

  // Connect to WiFi
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");
  //Server
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());

  // Initialize Firebase
  // Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    Firebase.begin(&config, NULL);

}

void loop() {
  //Added-START
  while (SerialGPS.available() > 0)
    if (gps.encode(SerialGPS.read())) {
      if (gps.location.isValid()) {
        Latitude = gps.location.lat();
        LatitudeString = String(Latitude, 6);
        Longitude = gps.location.lng();
        LongitudeString = String(Longitude, 6);
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
        hour = gps.time.hour() + 8;
        minute = gps.time.minute();


        if (hour < 10)
          TimeString = '0';
        TimeString += String(hour);
        TimeString += " : ";

        if (minute < 10)
          TimeString += '0';
        TimeString += String(minute);
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

  //Added- END

  if (Serial.available()) {
    // Read data from Arduino Uno
    String data = Serial.readStringUntil('\n');
    Serial.println(data);
    Serial.println(LatitudeString);

    // Create the JSON object
    firebaseJson.clear();
    firebaseJson.set("n", data.substring(data.indexOf("n:") + 2, data.indexOf(", p:")));
    firebaseJson.set("p", data.substring(data.indexOf("p:") + 2, data.indexOf(", k:")));
    firebaseJson.set("k", data.substring(data.indexOf("k:") + 2));
    firebaseJson.set("lat", LatitudeString);
    firebaseJson.set("long", LongitudeString);
    firebaseJson.set("date", DateString);
    firebaseJson.set("time", TimeString);
    // Send data to Firebase
    Firebase.pushJSON(firebaseData, "/NPK", firebaseJson);
  }
}
