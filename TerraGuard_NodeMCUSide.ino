#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>

// WiFi configuration
// WiFi configuration
const char* ssid = "GlobeAtHome_F8742";
const char* password = "3C901AC1";

// Firebase configuration
#define FIREBASE_HOST "https://temphum-4ede4-default-rtdb.firebaseio.com/" 
#define FIREBASE_AUTH "ZC8aQ7uHOT6n3AgQp2JQyDwClkK9nwXt1ddQ8v0a"

// Firebase object
FirebaseData firebaseData;
FirebaseJson firebaseJson;

void setup() {
  Serial.begin(9600);
  delay(100);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");

  // Initialize Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  if (Serial.available()) {
    // Read data from Arduino Uno
    String data = Serial.readStringUntil('\n');
    Serial.println(data);

    // Create the JSON object
    firebaseJson.clear();
    firebaseJson.set("n", data.substring(data.indexOf("n:") + 2, data.indexOf(", p:")));
    firebaseJson.set("p", data.substring(data.indexOf("p:") + 2, data.indexOf(", k:")));
    firebaseJson.set("k", data.substring(data.indexOf("k:") + 2));

    // Send data to Firebase
    Firebase.pushJSON(firebaseData, "/NPK", firebaseJson);
    }
   
  }


