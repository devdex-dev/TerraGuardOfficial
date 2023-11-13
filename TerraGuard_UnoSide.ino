#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>


// RE and DE Pins set the RS485 module
// to Receiver or Transmitter mode
#define RE 8
#define DE 7

// The serial connection to the GPS device

// Modbus RTU requests for reading NPK values
const byte nitro[] = { 0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c };
const byte phos[] = { 0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc };
const byte pota[] = { 0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0 };

// A variable used to store NPK values
byte values[11];

// Sets up a new SoftwareSerial object

SoftwareSerial mod(2, 3);
SoftwareSerial espSerial(1, 0);  // RX, TX

//SoftwareSerial mod(10, 11);

// Variables to store the previous values
byte prevVal1, prevVal2, prevVal3;

// Number of consecutive stable readings required
const int stableCount = 5;

// Counter variables to track stability
int stableVal1Count = 0;
int stableVal2Count = 0;
int stableVal3Count = 0;

void setup() {
  // Set the baud rate for the Serial port
  Serial.begin(9600);

  // Set the baud rate for the SerialSoftware object
  mod.begin(9600);

  // Define pin modes for RE and DE
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);

  pinMode(9, 1);   //red led
  pinMode(10, 1);  //green led
  pinMode(11, 1);  //blue led
}

void loop() {

  // Read values
  byte val1, val2, val3;
  val1 = nitrogen();
  delay(500);
  val2 = phosphorous();
  delay(500);
  val3 = potassium();
  delay(500);

  // Check if the values have stabilized
  if (val1 == prevVal1) {
    stableVal1Count++;
  } else {
    stableVal1Count = 0;
    prevVal1 = val1;
  }

  if (val2 == prevVal2) {
    stableVal2Count++;
  } else {
    stableVal2Count = 0;
    prevVal2 = val2;
  }

  if (val3 == prevVal3) {
    stableVal3Count++;
  } else {
    stableVal3Count = 0;
    prevVal3 = val3;
  }

 if ((val1 == 255 && val2 == 255 && val3 == 255) || (val1 == 0 && val2 == 0 && val3 == 0)) {
    // Keep LED red and do nothing
    analogWrite(9, 0);   // blue
    analogWrite(10, 0);  // green
    analogWrite(11, 255); // red
} else {
    // Existing logic
    analogWrite(9, 255);  // blue
    analogWrite(10, 0);   // green
    analogWrite(11, 0);   // red
    delay(1000);

    if (stableVal1Count >= stableCount && stableVal2Count >= stableCount && stableVal3Count >= stableCount) {
        Serial.print("n: ");
        Serial.print(val1);
        Serial.print(" mg/kg, p: ");
        Serial.print(val2);
        Serial.print(" mg/kg, k: ");
        Serial.print(val3);
        Serial.println(" mg/kg");

        delay(1000);
        analogWrite(9, 255);  // blue
        analogWrite(10, 0);   // green
        analogWrite(11, 255); // red

        exit(0);  // Terminate the code or loop
    }
}


byte nitrogen() {
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  if (mod.write(nitro, sizeof(nitro)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (byte i = 0; i < 7; i++) {
      //Serial.print(mod.read(),HEX);
      values[i] = mod.read();
      // Serial.print(values[i],HEX);
    }
    // Serial.println();
  }
  return values[4];
}

byte phosphorous() {
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  if (mod.write(phos, sizeof(phos)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (byte i = 0; i < 7; i++) {
      //Serial.print(mod.read(),HEX);
      values[i] = mod.read();
      // Serial.print(values[i],HEX);
    }
    // Serial.println();
  }
  return values[4];
}

byte potassium() {
  digitalWrite(DE, HIGH);
  digitalWrite(RE, HIGH);
  delay(10);
  if (mod.write(pota, sizeof(pota)) == 8) {
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
    for (byte i = 0; i < 7; i++) {
      //Serial.print(mod.read(),HEX);
      values[i] = mod.read();
      // Serial.print(values[i],HEX);
    }
    // Serial.println();
  }
  return values[4];
}
