#include <Wire.h>

// AT24C32 EEPROM Configuration
#define EEPROM_ADDR 0x50  // I2C address (try 0x50 if this doesn't work)

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  Serial.println(F("AT24C32 EEPROM Simple Test"));
  Serial.println(F("=========================="));
  
  // Test connectivity
  Wire.beginTransmission(EEPROM_ADDR);
  if (Wire.endTransmission() == 0) {
    Serial.println(F("✓ EEPROM Found!"));
  } else {
    Serial.println(F("✗ EEPROM Not Found! Check connections."));
    while(1);
  }
  
  // Simple write/read test
  Serial.println(F("\nTesting write/read..."));
  
  // Write test data
  writeByte(0, 0xAA);
  writeByte(1, 0x55);
  writeString(10, "Hello");
  
  delay(50); // Write delay
  
  // Read and verify
  if (readByte(0) == 0xAA && readByte(1) == 0x55) {
    Serial.println(F("✓ Byte test passed"));
  } else {
    Serial.println(F("✗ Byte test failed"));
  }
  
  if (readString(10, 5) == "Hello") {
    Serial.println(F("✓ String test passed"));
  } else {
    Serial.println(F("✗ String test failed"));
  }
  
  Serial.println(F("\nEEPROM is working!"));
  Serial.println(F("Commands: w(write), r(read)"));
}

void loop() {
  if (Serial.available()) {
    char cmd = Serial.read();
    
    if (cmd == 'w') {
      Serial.println(F("Enter address (0-4095):"));
      while(!Serial.available());
      int addr = Serial.parseInt();
      
      Serial.println(F("Enter text:"));
      while(!Serial.available());
      String text = Serial.readString();
      text.trim();
      
      writeString(addr, text);
      Serial.println(F("Written!"));
      
    } else if (cmd == 'r') {
      Serial.println(F("Enter address:"));
      while(!Serial.available());
      int addr = Serial.parseInt();
      
      Serial.println(F("Enter length:"));
      while(!Serial.available());  
      int len = Serial.parseInt();
      
      String data = readString(addr, len);
      Serial.print(F("Data: "));
      Serial.println(data);
    }
  }
}

void writeByte(int addr, byte data) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(addr >> 8);   // High byte
  Wire.write(addr & 0xFF); // Low byte
  Wire.write(data);
  Wire.endTransmission();
  delay(5);
}

byte readByte(int addr) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(addr >> 8);
  Wire.write(addr & 0xFF);
  Wire.endTransmission();
  
  Wire.requestFrom(EEPROM_ADDR, 1);
  return Wire.available() ? Wire.read() : 0;
}

void writeString(int addr, String str) {
  for (int i = 0; i < str.length(); i++) {
    writeByte(addr + i, str[i]);
  }
  writeByte(addr + str.length(), 0); // Null terminator
}

String readString(int addr, int maxLen) {
  String result = "";
  for (int i = 0; i < maxLen; i++) {
    byte b = readByte(addr + i);
    if (b == 0) break;
    result += (char)b;
  }
  return result;
}