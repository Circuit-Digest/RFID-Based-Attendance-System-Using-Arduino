/*
 * RFID Card Reader/Writer with Mode Selection
 * 
 * FEATURES:
 * - Read/Write name (max 32 characters) and ID (max 8 characters) to RFID cards
 * - Erase card data (clear name and ID)
 * - Mode selection system - user must choose operation before scanning card
 * - Block-based data storage with authentication
 * - Serial monitor interface for user interaction
 * 
 * APPLICATIONS:
 * - Employee ID card system
 * - Student identification cards
 * - Access control systems
 * - Inventory management tags
 * - Personal identification projects
 * 
 * HOW IT WORKS:
 * 1. User selects mode (Write/Read/Erase) via Serial Monitor
 * 2. System prompts to place RFID card near reader
 * 3. Card is detected and UID is displayed
 * 4. Selected operation is performed (write data, read data, or erase)
 * 5. System resets and asks for next mode selection
 *
 *
 * AUTHOR: CircuitDigest/me_RK
 * VERSION: 1.0
 */

#include <SPI.h>         // SPI communication library
#include <MFRC522.h>     // RFID reader library

// Pin definitions - connect RFID module to these pins
#define RST_PIN         9    // Reset pin for RFID module
#define SS_PIN          10   // Slave Select pin for SPI communication

// Data storage configuration - adjust these values as needed
const uint8_t NAME_LENGTH = 32;     // Maximum characters for name field
const uint8_t ID_LENGTH = 8;        // Maximum characters for ID field
const uint8_t NAME_START_BLOCK = 4; // RFID block where name storage begins
const uint8_t ID_START_BLOCK = 8;   // RFID block where ID storage begins

// Calculate how many RFID blocks needed (each block holds 16 bytes)
const uint8_t NAME_BLOCKS = (NAME_LENGTH + 15) / 16; // 2 blocks for 32 chars
const uint8_t ID_BLOCKS = (ID_LENGTH + 15) / 16;     // 1 block for 8 chars

// Create RFID reader object
MFRC522 mfrc522(SS_PIN, RST_PIN);

// Default authentication key for RFID cards (all cards start with this key)
MFRC522::MIFARE_Key key;

// Global variable to store selected operation mode
char selectedMode = '\0';  // '\0' means no mode selected

void setup() {
  // Initialize serial communication for user interaction
  Serial.begin(9600);
  while (!Serial);  // Wait for serial port to connect
  
  // Initialize SPI bus and RFID reader
  SPI.begin();
  mfrc522.PCD_Init();
  
  // Set up default authentication key (FF FF FF FF FF FF)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  
  // Display startup message and show available commands
  Serial.println("RFID Name/ID Reader/Writer Ready");
  promptForMode();
}

void loop() {
  // STEP 1: Check if user has selected a mode
  if (selectedMode == '\0') {
    // No mode selected - wait for user to choose operation
    if (Serial.available() > 0) {
      char command = Serial.read();
      
      // Clear any extra characters from serial buffer
      while (Serial.available() > 0) {
        Serial.read();
      }
      
      // Process user's command
      switch (command) {
        case 'w':
        case 'W':
          selectedMode = 'w';  // Set write mode
          Serial.println("WRITE mode selected. Now place your card near the reader...");
          break;
        case 'r':
        case 'R':
          selectedMode = 'r';  // Set read mode
          Serial.println("READ mode selected. Now place your card near the reader...");
          break;
        case 'e':
        case 'E':
          selectedMode = 'e';  // Set erase mode
          Serial.println("ERASE mode selected. Now place your card near the reader...");
          break;
        default:
          // Invalid command entered
          Serial.println("Invalid command. Please enter 'w' for write, 'r' for read, or 'e' for erase.");
          break;
      }
    }
    return; // Don't scan for cards until mode is selected
  }
  
  // STEP 2: Mode is selected - now scan for RFID cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;  // No card detected, keep waiting
  }

  // STEP 3: Select and read the detected card
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;  // Failed to read card, try again
  }

  // STEP 4: Card successfully detected - show card information
  Serial.println("\nCard detected!");
  Serial.print("Card UID: ");
  printHex(mfrc522.uid.uidByte, mfrc522.uid.size);  // Display unique card ID
  Serial.println();

  // STEP 5: Execute the operation based on selected mode
  switch (selectedMode) {
    case 'w':
      writeCardData();   // Write name and ID to card
      break;
    case 'r':
      readCardData();    // Read name and ID from card
      break;
    case 'e':
      eraseCardData();   // Erase data from card
      break;
  }

  // STEP 6: Clean up after operation
  mfrc522.PICC_HaltA();         // Stop communication with card
  mfrc522.PCD_StopCrypto1();    // Stop encryption
  
  // STEP 7: Reset system for next operation
  selectedMode = '\0';           // Clear selected mode
  Serial.println("\nOperation completed!");
  promptForMode();               // Ask user to select next operation
}

// Function to display available commands to user
void promptForMode() {
  Serial.println("\n=== Select Mode ===");
  Serial.println("Commands:");
  Serial.println("  w - Write data to card");
  Serial.println("  r - Read data from card");
  Serial.println("  e - Erase data from card");
  Serial.print("Enter command (w/r/e): ");
}

// Function to write name and ID data to RFID card
void writeCardData() {
  String name, id;  // Variables to store user input
  
  Serial.println("\n=== WRITE MODE ===");
  
  // Get name from user
  Serial.print("Enter name (max " + String(NAME_LENGTH) + " chars): ");
  while (Serial.available() == 0) {}  // Wait for user input
  name = Serial.readStringUntil('\n');
  name.trim();  // Remove extra spaces
  
  // Get ID from user
  Serial.print("Enter ID (max " + String(ID_LENGTH) + " chars): ");
  while (Serial.available() == 0) {}  // Wait for user input
  id = Serial.readStringUntil('\n');
  id.trim();  // Remove extra spaces
  
  // Check if inputs are not too long
  if (name.length() > NAME_LENGTH) {
    Serial.println("Error: Name too long!");
    return;
  }
  
  if (id.length() > ID_LENGTH) {
    Serial.println("Error: ID too long!");
    return;
  }
  
  Serial.println("Writing to card...");
  
  // Write name to card blocks
  if (writeStringToBlocks(name, NAME_START_BLOCK, NAME_BLOCKS, NAME_LENGTH)) {
    Serial.println("Name written successfully!");
  } else {
    Serial.println("Failed to write name!");
    return;
  }
  
  // Write ID to card blocks
  if (writeStringToBlocks(id, ID_START_BLOCK, ID_BLOCKS, ID_LENGTH)) {
    Serial.println("ID written successfully!");
  } else {
    Serial.println("Failed to write ID!");
    return;
  }
  
  Serial.println("Data written to card successfully!");
}

// Function to read name and ID data from RFID card
void readCardData() {
  Serial.println("\n=== READ MODE ===");
  
  // Read name from card blocks
  String name = readStringFromBlocks(NAME_START_BLOCK, NAME_BLOCKS, NAME_LENGTH);
  if (name != "") {
    Serial.println("Name: " + name);
  } else {
    Serial.println("Failed to read name!");
  }
  
  // Read ID from card blocks
  String id = readStringFromBlocks(ID_START_BLOCK, ID_BLOCKS, ID_LENGTH);
  if (id != "") {
    Serial.println("ID: " + id);
  } else {
    Serial.println("Failed to read ID!");
  }
}

// Function to erase all data from RFID card
void eraseCardData() {
  Serial.println("\n=== ERASE MODE ===");
  
  // Create empty strings to overwrite existing data
  String emptyName = "";
  String emptyId = "";
  
  Serial.println("Erasing card data...");
  
  // Overwrite name blocks with empty data
  if (writeStringToBlocks(emptyName, NAME_START_BLOCK, NAME_BLOCKS, NAME_LENGTH)) {
    Serial.println("Name erased successfully!");
  } else {
    Serial.println("Failed to erase name!");
    return;
  }
  
  // Overwrite ID blocks with empty data
  if (writeStringToBlocks(emptyId, ID_START_BLOCK, ID_BLOCKS, ID_LENGTH)) {
    Serial.println("ID erased successfully!");
  } else {
    Serial.println("Failed to erase ID!");
    return;
  }
  
  Serial.println("Card data erased successfully!");
}

// Function to write string data to multiple RFID blocks
bool writeStringToBlocks(String data, uint8_t startBlock, uint8_t numBlocks, uint8_t maxLength) {
  // Pad string with null characters to fill the allocated space
  while (data.length() < maxLength) {
    data += '\0';
  }
  
  // Write data block by block (each block holds 16 bytes)
  for (uint8_t i = 0; i < numBlocks; i++) {
    uint8_t block = startBlock + i;
    
    // Skip trailer blocks (they contain authentication keys, not data)
    if ((block + 1) % 4 == 0) {
      Serial.println("Skipping trailer block " + String(block));
      continue;
    }
    
    // Prepare 16-byte buffer for this block
    byte buffer[16];
    for (int j = 0; j < 16; j++) {
      int charIndex = i * 16 + j;  // Calculate position in string
      if (charIndex < data.length()) {
        buffer[j] = data.charAt(charIndex);  // Copy character
      } else {
        buffer[j] = 0;  // Fill remaining space with zeros
      }
    }
    
    // Authenticate before writing (required for security)
    if (!authenticateBlock(block)) {
      return false;
    }
    
    // Write the 16-byte buffer to the block
    MFRC522::StatusCode status = mfrc522.MIFARE_Write(block, buffer, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.println("Write failed for block " + String(block) + ": " + String(mfrc522.GetStatusCodeName(status)));
      return false;
    }
  }
  
  return true;  // All blocks written successfully
}

// Function to read string data from multiple RFID blocks
String readStringFromBlocks(uint8_t startBlock, uint8_t numBlocks, uint8_t maxLength) {
  String result = "";  // String to store the read data
  
  // Read data block by block
  for (uint8_t i = 0; i < numBlocks; i++) {
    uint8_t block = startBlock + i;
    
    // Skip trailer blocks (they don't contain our data)
    if ((block + 1) % 4 == 0) {
      continue;
    }
    
    // Authenticate before reading (required for security)
    if (!authenticateBlock(block)) {
      return "";  // Authentication failed
    }
    
    // Read 16 bytes from the block
    byte buffer[18];  // Extra bytes for safety
    byte size = sizeof(buffer);
    MFRC522::StatusCode status = mfrc522.MIFARE_Read(block, buffer, &size);
    
    if (status != MFRC522::STATUS_OK) {
      Serial.println("Read failed for block " + String(block) + ": " + String(mfrc522.GetStatusCodeName(status)));
      return "";
    }
    
    // Convert bytes to string (16 bytes per block)
    for (int j = 0; j < 16; j++) {
      if (result.length() >= maxLength) break;  // Don't exceed max length
      if (buffer[j] != 0) {  // Only add non-null characters
        result += (char)buffer[j];
      }
    }
  }
  
  return result;
}

// Function to authenticate access to a specific block
bool authenticateBlock(uint8_t block) {
  // Calculate which sector this block belongs to
  uint8_t sector = block / 4;  // Each sector has 4 blocks
  uint8_t trailerBlock = sector * 4 + 3;  // Last block of sector contains keys
  
  // Authenticate using key A (default key)
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
    MFRC522::PICC_CMD_MF_AUTH_KEY_A,  // Use key A
    trailerBlock,                      // Authenticate to sector's trailer block
    &key,                             // Authentication key
    &(mfrc522.uid)                    // Card's unique ID
  );
  
  if (status != MFRC522::STATUS_OK) {
    Serial.println("Authentication failed for sector " + String(sector) + ": " + String(mfrc522.GetStatusCodeName(status)));
    return false;
  }
  
  return true;  // Authentication successful
}

// Function to display bytes in hexadecimal format (for showing card UID)
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");  // Add leading zero if needed
    Serial.print(buffer[i], HEX);  // Print in hexadecimal
  }
}

// Utility function to change configuration at runtime (for advanced users)
void updateConfig(uint8_t nameLen, uint8_t idLen, uint8_t nameBlock, uint8_t idBlock) {
  // Note: Configuration variables are const, so this function is for reference only
  Serial.println("Configuration updated:");
  Serial.println("Name Length: " + String(nameLen));
  Serial.println("ID Length: " + String(idLen));
  Serial.println("Name Start Block: " + String(nameBlock));
  Serial.println("ID Start Block: " + String(idBlock));
}