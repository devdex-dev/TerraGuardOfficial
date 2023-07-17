#include <SoftwareSerial.h>
#include <Wire.h>

// RE and DE Pins set the RS485 module
// to Receiver or Transmitter mode
#define RE 8
#define DE 7


// Modbus RTU requests for reading NPK values
const byte nitro[] = {0x01,0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01,0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01,0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

// A variable used to store NPK values
byte values[11];

// Sets up a new SoftwareSerial object

SoftwareSerial mod(2, 3);
SoftwareSerial espSerial(1, 0); // RX, TX
//SoftwareSerial mod(10, 11);

void setup() {
  // Set the baud rate for the Serial port
  Serial.begin(9600);
  
  espSerial.begin(9600);
  delay(1000); // Delay for NodeMCU initialization
  // Set the baud rate for the SerialSoftware object
  mod.begin(9600);

  // Define pin modes for RE and DE
  pinMode(RE, OUTPUT);
  pinMode(DE, OUTPUT);
  
  delay(500);
}
 
void loop() {
   
  // Read values
  byte val1,val2,val3;
  val1 = nitrogen();
  delay(500);
  val2 = phosphorous();
   delay(500);
  val3 = potassium();
  delay(500);

 // Print values to the serial monitor -OLD
  Serial.print("n: ");
  Serial.print(String(val1));
  Serial.print(" mg/kg");
  
  Serial.print(", p: ");
  Serial.print(String(val2));
  Serial.print(" mg/kg");
 
  Serial.print(", k:");
  Serial.print(String(val3));
  Serial.println(" mg/kg");
 
  
   }

 
byte nitrogen(){
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  if(mod.write(nitro,sizeof(nitro))==8){
    digitalWrite(DE,LOW);
    digitalWrite(RE,LOW);
    for(byte i=0;i<7;i++){
    //Serial.print(mod.read(),HEX);
    values[i] = mod.read();
    // Serial.print(values[i],HEX);
    }
    // Serial.println();
  }
  return values[4];
}
 
byte phosphorous(){
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  if(mod.write(phos,sizeof(phos))==8){
    digitalWrite(DE,LOW);
    digitalWrite(RE,LOW);
    for(byte i=0;i<7;i++){
    //Serial.print(mod.read(),HEX);
    values[i] = mod.read();
    // Serial.print(values[i],HEX);
    }
    // Serial.println();
  }
  return values[4];
}
 
byte potassium(){
  digitalWrite(DE,HIGH);
  digitalWrite(RE,HIGH);
  delay(10);
  if(mod.write(pota,sizeof(pota))==8){
    digitalWrite(DE,LOW);
    digitalWrite(RE,LOW);
    for(byte i=0;i<7;i++){
    //Serial.print(mod.read(),HEX);
    values[i] = mod.read();
    // Serial.print(values[i],HEX);
    }
    // Serial.println();
  }
  return values[4];
}