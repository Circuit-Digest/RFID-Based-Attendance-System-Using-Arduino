/*
 * ===============================================
 * RFID-BASED ATTENDANCE MANAGEMENT SYSTEM
 * ===============================================
 * 
 * FEATURES:
 * - RFID card scanning for check-in/check-out
 * - Real-time clock (RTC) for timestamp logging
 * - LCD display with menu navigation
 * - EEPROM storage for persistent data
 * - Automatic IN/OUT status tracking
 * - View attendance logs on LCD/Serial
 * - Inside count tracking
 * - Scrolling text for long names
 * - Button-based menu system
 * 
 * APPLICATIONS:
 * - Office attendance tracking
 * - School/college attendance
 * - Event check-in systems
 * - Access control systems
 * - Workshop/lab entry tracking
 * 
 * HARDWARE REQUIREMENTS:
 * - Arduino Uno/Nano
 * - MFRC522 RFID Reader
 * - DS1307 RTC Module
 * - 16x2 I2C LCD Display
 * - AT24C32 EEPROM (32KB)
 * - 2 Push buttons (Menu & Select)
 * - RFID cards/tags
 * 
 * WORKING PRINCIPLE:
 * 1. System reads RFID cards and extracts stored name/ID
 * 2. Checks last status (IN/OUT) and toggles accordingly
 * 3. Logs entry with timestamp to EEPROM
 * 4. Updates inside count and displays on LCD
 * 5. Provides menu interface for viewing logs and status
 * 
 * PIN CONNECTIONS:
 * - RFID: RST=9, SS=10, MOSI=11, MISO=12, SCK=13
 * - LCD & RTC & EEPROM: SDA=A4, SCL=A5
 * - Buttons: Menu=2, Select=3
 * 
 * AUTHOR: CircuitDigest/me_RK
 * VERSION: 1.0
 */

// Include required libraries
#include <SPI.h>           // For RFID communication
#include <MFRC522.h>       // RFID reader library
#include <Wire.h>          // I2C communication
#include "RTClib.h"        // Real-time clock library
#include <LiquidCrystal_I2C.h> // LCD display library

// ===============================================
// PIN DEFINITIONS
// ===============================================
#define RST_PIN 7          // RFID reset pin
#define SS_PIN 10          // RFID slave select pin
#define BUTTON_MENU 8      // Menu navigation button
#define BUTTON_SELECT 9    // Selection/confirmation button

// ===============================================
// HARDWARE COMPONENT INITIALIZATION
// ===============================================
#define EEPROM_ADDR 0x50   // EEPROM I2C address
MFRC522 mfrc522(SS_PIN, RST_PIN);        // RFID reader object
RTC_DS1307 rtc;                          // Real-time clock object
LiquidCrystal_I2C lcd(0x27, 16, 2);     // LCD object (address, columns, rows)
MFRC522::MIFARE_Key key;                 // RFID authentication key

// ===============================================
// RFID CARD DATA CONFIGURATION
// ===============================================
#define NAME_START_BLOCK 4    // Starting block for name storage on card
#define ID_START_BLOCK 8      // Starting block for ID storage on card
#define NAME_LENGTH 32        // Maximum name length
#define ID_LENGTH 8           // Maximum ID length

// ===============================================
// DATA STORAGE CONFIGURATION
// ===============================================
#define MAX_LOGS 30          // Maximum number of log entries
#define LOG_SIZE 80          // Size of each log entry in bytes
int logCount = 0;            // Current number of stored logs
int insideCount = 0;         // Number of people currently inside

// ===============================================
// LCD MENU SYSTEM VARIABLES
// ===============================================
// Menu states for navigation
enum MenuState {
  MENU_HOME,        // Home screen with time and inside count
  MENU_VIEW_LOGS,   // View attendance logs
  MENU_STATUS,      // Show system status
  MENU_CLEAR_LOGS,  // Clear all logs
  MENU_SET_TIME     // Set RTC time
};

MenuState currentMenu = MENU_HOME;    // Current active menu
int menuIndex = 0;                    // Current menu item index
unsigned long lastButtonPress = 0;    // Debounce timer for buttons
unsigned long lastLCDUpdate = 0;      // Timer for LCD updates
unsigned long homeScreenTimer = 0;    // Timer for home screen refresh
bool inSubMenu = false;               // Flag for submenu navigation
int logViewIndex = 0;                 // Index for viewing logs

// ===============================================
// TEXT SCROLLING VARIABLES
// ===============================================
unsigned long lastScrollTime = 0;     // Timer for scrolling animation
int scrollPosition = 0;               // Current scroll position
String currentScrollText = "";        // Text being scrolled
bool isScrolling = false;             // Scrolling active flag
#define SCROLL_DELAY 500              // Milliseconds between scroll steps
#define SCROLL_PAUSE 2000             // Pause duration at start/end

// ===============================================
// BUTTON HANDLING VARIABLES
// ===============================================
bool menuButtonPressed = false;       // Menu button state tracking
bool selectButtonPressed = false;     // Select button state tracking

// ===============================================
// INITIAL SETUP FUNCTION
// ===============================================
void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  while (!Serial);  // Wait for serial connection
  
  // Configure button pins with internal pull-up resistors
  pinMode(BUTTON_MENU, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  
  // Initialize and configure LCD display
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("RFID Attendance");
  lcd.setCursor(0, 1);
  lcd.print("Starting up...");
  delay(2000);
  
  // Initialize SPI communication for RFID
  SPI.begin();
  mfrc522.PCD_Init();
  
  // Initialize I2C communication
  Wire.begin();
  
  // Initialize Real-Time Clock
  if (!rtc.begin()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC Error!");
    Serial.println(F("RTC Error!"));
    while(1);  // Stop execution if RTC fails
  }
  
  // Initialize RFID authentication key (default key)
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  
  // Read stored data from EEPROM
  logCount = readInt(0);      // Read log count from address 0
  insideCount = readInt(2);   // Read inside count from address 2
  
  // Validate read data and reset if corrupted
  if (logCount < 0 || logCount > MAX_LOGS) logCount = 0;
  if (insideCount < 0) insideCount = 0;
  
  // Display system ready message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Ready!");
  lcd.setCursor(0, 1);
  lcd.print("Inside: ");
  lcd.print(insideCount);
  delay(2000);
  
  // Print startup information to serial monitor
  Serial.println(F("\n=== RFID IN/OUT System Ready ==="));
  Serial.print(F("Total logs: "));
  Serial.println(logCount);
  Serial.print(F("People inside: "));
  Serial.println(insideCount);
  Serial.println(F("Commands: v(view), c(clear), s(status), t(time)"));
  Serial.println(F("Scan card for IN/OUT..."));
  
  // Show home screen
  displayHomeScreen();
}

// ===============================================
// MAIN PROGRAM LOOP
// ===============================================
void loop() {
  handleButtons();        // Process button presses
  handleSerialCommands(); // Process serial commands
  handleLCDUpdates();     // Update LCD display
  handleRFIDScan();       // Check for RFID cards
  handleScrolling();      // Handle text scrolling animation
}

// ===============================================
// TEXT SCROLLING FUNCTIONS
// ===============================================

// Handle scrolling animation for long text
void handleScrolling() {
  // Exit if not scrolling or text fits on screen
  if (!isScrolling || currentScrollText.length() <= 16) return;
  
  unsigned long currentTime = millis();
  
  // Check if it's time to scroll to next position
  if (currentTime - lastScrollTime >= SCROLL_DELAY) {
    scrollPosition++;
    
    // Reset scroll position when reaching end
    if (scrollPosition > currentScrollText.length() - 16) {
      delay(SCROLL_PAUSE);  // Pause at end
      scrollPosition = 0;   // Reset to beginning
      delay(SCROLL_PAUSE);  // Pause at beginning
    }
    
    // Update LCD display with scrolled text
    lcd.setCursor(0, 1);
    lcd.print("                "); // Clear the line
    lcd.setCursor(0, 1);
    
    // Extract text to display (16 characters max)
    String displayText = currentScrollText.substring(scrollPosition);
    if (displayText.length() > 16) {
      displayText = displayText.substring(0, 16);
    }
    lcd.print(displayText);
    
    lastScrollTime = currentTime;
  }
}

// Start scrolling animation for given text
void startScrolling(String text) {
  currentScrollText = text;
  scrollPosition = 0;
  isScrolling = true;
  lastScrollTime = millis();
}

// Stop scrolling animation
void stopScrolling() {
  isScrolling = false;
  currentScrollText = "";
  scrollPosition = 0;
}

// ===============================================
// BUTTON HANDLING FUNCTIONS
// ===============================================

// Process button presses with debouncing
void handleButtons() {
  unsigned long currentTime = millis();
  
  // Implement button debouncing (200ms minimum between presses)
  if (currentTime - lastButtonPress < 200) return;
  
  // Handle Menu Button Press
  if (digitalRead(BUTTON_MENU) == LOW && !menuButtonPressed) {
    menuButtonPressed = true;
    lastButtonPress = currentTime;
    
    if (inSubMenu) {
      // Exit from submenu back to home
      inSubMenu = false;
      logViewIndex = 0;
      stopScrolling();
      displayHomeScreen();
    } else {
      // Navigate through main menu options
      menuIndex = (menuIndex + 1) % 5;  // Cycle through 5 menu items
      displayMenu();
    }
  } else if (digitalRead(BUTTON_MENU) == HIGH) {
    menuButtonPressed = false;  // Reset button state when released
  }
  
  // Handle Select Button Press
  if (digitalRead(BUTTON_SELECT) == LOW && !selectButtonPressed) {
    selectButtonPressed = true;
    lastButtonPress = currentTime;
    
    if (!inSubMenu) {
      // Execute selected menu action
      executeMenuAction();
    } else if (currentMenu == MENU_VIEW_LOGS) {
      // Navigate through log entries
      stopScrolling();
      logViewIndex = (logViewIndex + 1) % max(1, logCount);
      displayLogEntry(logViewIndex);
    }
  } else if (digitalRead(BUTTON_SELECT) == HIGH) {
    selectButtonPressed = false;  // Reset button state when released
  }
}

// ===============================================
// SERIAL COMMAND HANDLING
// ===============================================

// Process commands received via serial monitor
void handleSerialCommands() {
  if (Serial.available()) {
    char cmd = Serial.read();
    while(Serial.available()) Serial.read();  // Clear buffer
    
    // Execute command based on received character
    switch(cmd) {
      case 'v': viewLogs(); break;        // View all logs
      case 'c': clearLogs(); break;       // Clear all logs
      case 's': showStatus(); break;      // Show current status
      case 't': setRTCTime(); break;      // Set RTC time
    }
  }
}

// ===============================================
// LCD UPDATE FUNCTIONS
// ===============================================

// Handle periodic LCD updates
void handleLCDUpdates() {
  unsigned long currentTime = millis();
  
  // Update home screen periodically or after inactivity
  if (currentMenu == MENU_HOME && 
      (currentTime - lastLCDUpdate > 30000 || 
       (currentTime - lastButtonPress > 10000 && currentTime - homeScreenTimer > 5000))) {
    displayHomeScreen();
    homeScreenTimer = currentTime;
  }
}

// ===============================================
// RFID CARD SCANNING FUNCTIONS
// ===============================================

// Main RFID card detection and processing function
void handleRFIDScan() {
  // Check if a new card is present and readable
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  // Display scanning message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanning card...");
  
  Serial.println(F("\n--- Card Detected ---"));
  
  // Read stored data from RFID card
  String cardName = readCardString(NAME_START_BLOCK, NAME_LENGTH);
  String cardID = readCardString(ID_START_BLOCK, ID_LENGTH);
  
  // Display read data in serial monitor
  Serial.print(F("Name: "));
  Serial.println(cardName.length() > 0 ? cardName : F("Empty"));
  Serial.print(F("ID: "));
  Serial.println(cardID.length() > 0 ? cardID : F("Empty"));
  
  // Validate card data (both name and ID must be present)
  if (cardName.length() == 0 || cardID.length() == 0) {
    // Card is not properly initialized - deny access
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ACCESS DENIED");
    lcd.setCursor(0, 1);
    lcd.print("Invalid Card");
    
    Serial.println(F("✗ ACCESS DENIED - Card not initialized"));
    Serial.println(F("Card must have both Name and ID data"));
    
    delay(3000);
    displayHomeScreen();
    
    // Stop card communication
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }
  
  // Card has valid data - proceed with access
  Serial.println(F("✓ Valid card detected"));
  
  // Determine new status based on last recorded status
  String lastStatus = getLastStatus(cardID);
  String newStatus = (lastStatus == "IN") ? "OUT" : "IN";
  
  // Log the entry with current timestamp
  logEntry(cardID, cardName, newStatus);
  
  // Update inside count based on new status
  if (newStatus == "IN") {
    insideCount++;
  } else {
    insideCount--;
    if (insideCount < 0) insideCount = 0;  // Prevent negative count
  }
  
  // Save updated inside count to EEPROM
  writeInt(2, insideCount);
  
  // Display result on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  if (cardName.length() > 16) {
    lcd.print(cardName.substring(0, 16));  // Truncate long names
  } else {
    lcd.print(cardName);
  }
  
  lcd.setCursor(0, 1);
  lcd.print(newStatus == "IN" ? "CHECKED IN" : "CHECKED OUT");
  
  // Print result to serial monitor
  Serial.print(F("✓ "));
  Serial.println(newStatus == "IN" ? "CHECKED IN" : "CHECKED OUT");
  Serial.print(F("People inside: "));
  Serial.println(insideCount);
  
  delay(3000);  // Show result for 3 seconds
  displayHomeScreen();
  
  // Stop card communication
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

// ===============================================
// LCD DISPLAY FUNCTIONS
// ===============================================

// Display main home screen with time and inside count
void displayHomeScreen() {
  currentMenu = MENU_HOME;
  lastLCDUpdate = millis();
  stopScrolling();
  
  lcd.clear();
  
  // Top line: Current time and date
  DateTime now = rtc.now();
  lcd.setCursor(0, 0);
  if (now.hour() < 10) lcd.print("0");    // Add leading zero
  lcd.print(now.hour());
  lcd.print(":");
  if (now.minute() < 10) lcd.print("0");  // Add leading zero
  lcd.print(now.minute());
  
  lcd.print("  ");
  lcd.print(now.day());
  lcd.print("/");
  lcd.print(now.month());
  lcd.print("/");
  lcd.print(now.year());
  
  // Bottom line: Inside count and scan prompt
  lcd.setCursor(0, 1);
  lcd.print("Inside:");
  lcd.print(insideCount);
  lcd.print(" SCAN>");
}

// Display menu options
void displayMenu() {
  stopScrolling();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MENU:");
  
  lcd.setCursor(0, 1);
  // Display current menu option
  switch(menuIndex) {
    case 0:
      lcd.print("> View Logs");
      break;
    case 1:
      lcd.print("> Status");
      break;
    case 2:
      lcd.print("> Clear Logs");
      break;
    case 3:
      lcd.print("> Set Time");
      break;
    case 4:
      lcd.print("> Home");
      break;
  }
}

// Execute selected menu action
void executeMenuAction() {
  switch(menuIndex) {
    case 0: // View Logs
      currentMenu = MENU_VIEW_LOGS;
      inSubMenu = true;
      logViewIndex = 0;
      displayLogEntry(0);
      break;
      
    case 1: // Status
      displayStatus();
      delay(3000);
      displayHomeScreen();
      break;
      
    case 2: // Clear Logs
      displayClearConfirm();
      break;
      
    case 3: // Set Time
      displaySetTimePrompt();
      break;
      
    case 4: // Home
      displayHomeScreen();
      break;
  }
}

// Display individual log entry with scrolling for long names
void displayLogEntry(int index) {
  stopScrolling();
  
  // Handle empty log case
  if (logCount == 0) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No logs found");
    lcd.setCursor(0, 1);
    lcd.print("MENU: Back");
    return;
  }
  
  // Read log entry from EEPROM
  String log = readLogEntry(index);
  if (log.length() == 0) return;
  
  // Parse log format: CardID|Name|Status|DateTime
  int pos1 = log.indexOf('|');
  int pos2 = log.indexOf('|', pos1 + 1);
  int pos3 = log.indexOf('|', pos2 + 1);
  
  if (pos1 > 0 && pos2 > pos1 && pos3 > pos2) {
    String cardID = log.substring(0, pos1);
    String name = log.substring(pos1 + 1, pos2);
    String status = log.substring(pos2 + 1, pos3);
    String dateTime = log.substring(pos3 + 1);
    
    lcd.clear();
    
    // First line: Entry number, Date (DD/MM), Time, Status
    lcd.setCursor(0, 0);
    lcd.print(String(index + 1));
    lcd.print(".");
    
    // Extract and format date/time
    int spacePos = dateTime.indexOf(' ');
    if (spacePos > 0) {
      String datePart = dateTime.substring(0, spacePos);
      String timePart = dateTime.substring(spacePos + 1);
      
      // Remove year from date display
      int lastSlash = datePart.lastIndexOf('/');
      String dateNoYear = "";
      if (lastSlash > 0) {
        dateNoYear = datePart.substring(0, lastSlash);
      }
      
      // Display formatted information
      lcd.print(dateNoYear);
      lcd.print(" ");
      lcd.print(timePart);
      lcd.print(" ");
      lcd.print(status);
    }
    
    // Second line: Name and Roll Number with scrolling support
    lcd.setCursor(0, 1);
    String secondLine = name + " (" + cardID + ")";
    
    if (secondLine.length() > 16) {
      // Start scrolling for long text
      startScrolling(secondLine);
      lcd.print(secondLine.substring(0, 16));
    } else {
      // Display complete text if it fits
      lcd.print(secondLine);
    }
    
    // Show navigation instructions briefly
    delay(1000);
    if (secondLine.length() <= 16) {
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print("SEL:Next MEN:Back");
      delay(1500);
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      lcd.print(secondLine);
    }
  }
}

// Display system status information
void displayStatus() {
  stopScrolling();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Inside: ");
  lcd.print(insideCount);
  
  lcd.setCursor(0, 1);
  lcd.print("Logs: ");
  lcd.print(logCount);
  lcd.print("/");
  lcd.print(MAX_LOGS);
}

// Display confirmation dialog for clearing logs
void displayClearConfirm() {
  stopScrolling();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Clear all logs?");
  lcd.setCursor(0, 1);
  lcd.print("SELECT: Yes");
  
  // Wait for user confirmation
  delay(500);
  unsigned long startTime = millis();
  while (millis() - startTime < 5000) {  // 5 second timeout
    if (digitalRead(BUTTON_SELECT) == LOW) {
      clearLogs();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Logs cleared!");
      delay(2000);
      displayHomeScreen();
      return;
    }
    if (digitalRead(BUTTON_MENU) == LOW) {
      displayHomeScreen();
      return;
    }
  }
  displayHomeScreen();  // Timeout - return to home
}

// Display instruction for setting time via serial
void displaySetTimePrompt() {
  stopScrolling();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Use Serial Mon.");
  lcd.setCursor(0, 1);
  lcd.print("to set time");
  delay(3000);
  displayHomeScreen();
}

// ===============================================
// RFID CARD DATA READING FUNCTIONS
// ===============================================

// Read string data from RFID card blocks
String readCardString(int startBlock, int maxLen) {
  String result = "";
  int blocksNeeded = (maxLen + 15) / 16;  // Calculate blocks needed
  
  for (int i = 0; i < blocksNeeded; i++) {
    int block = startBlock + i;
    
    // Skip trailer blocks (every 4th block)
    if ((block + 1) % 4 == 0) continue;
    
    // Authenticate with the sector
    int sector = block / 4;
    int trailerBlock = sector * 4 + 3;
    
    MFRC522::StatusCode status = mfrc522.PCD_Authenticate(
      MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    
    if (status != MFRC522::STATUS_OK) continue;
    
    // Read the block data
    byte buffer[18];
    byte size = sizeof(buffer);
    status = mfrc522.MIFARE_Read(block, buffer, &size);
    
    if (status == MFRC522::STATUS_OK) {
      // Extract characters from buffer
      for (int j = 0; j < 16 && result.length() < maxLen; j++) {
        if (buffer[j] != 0) {  // Skip null characters
          result += (char)buffer[j];
        }
      }
    }
  }
  
  result.trim();  // Remove whitespace
  return result;
}

// ===============================================
// TIME AND STATUS FUNCTIONS
// ===============================================

// Set RTC time via serial input
void setRTCTime() {
  Serial.println(F("\n=== Set RTC Time ==="));
  Serial.println(F("Enter year (e.g. 2024):"));
  while(!Serial.available());
  int year = Serial.parseInt();
  
  Serial.println(F("Enter month (1-12):"));
  while(!Serial.available());
  int month = Serial.parseInt();
  
  Serial.println(F("Enter day (1-31):"));
  while(!Serial.available());
  int day = Serial.parseInt();
  
  Serial.println(F("Enter hour (0-23):"));
  while(!Serial.available());
  int hour = Serial.parseInt();
  
  Serial.println(F("Enter minute (0-59):"));
  while(!Serial.available());
  int minute = Serial.parseInt();
  
  Serial.println(F("Enter second (0-59):"));
  while(!Serial.available());
  int second = Serial.parseInt();
  
  // Update RTC with new time
  rtc.adjust(DateTime(year, month, day, hour, minute, second));
  
  Serial.println(F("✓ RTC time updated!"));
  DateTime now = rtc.now();
  Serial.print(F("New time: "));
  printDateTime(now);
  Serial.println();
}

// Get card UID as hexadecimal string
String getCardUID() {
  String cardID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) cardID += "0";
    cardID += String(mfrc522.uid.uidByte[i], HEX);
  }
  cardID.toUpperCase();
  return cardID;
}

// Get last recorded status (IN/OUT) for a card
String getLastStatus(String cardID) {
  // Search through recent logs for this card
  for (int i = logCount - 1; i >= 0; i--) {
    String logEntry = readLogEntry(i);
    if (logEntry.indexOf(cardID) >= 0) {
      if (logEntry.indexOf("|IN|") >= 0) return "IN";
      if (logEntry.indexOf("|OUT|") >= 0) return "OUT";
    }
  }
  return "OUT"; // Default to OUT if no history found
}

// ===============================================
// DATA LOGGING FUNCTIONS
// ===============================================

// Create new log entry and store in EEPROM
void logEntry(String cardID, String name, String status) {
  // Handle log overflow by removing oldest entry
  if (logCount >= MAX_LOGS) {
    Serial.println(F("Log full! Clearing oldest..."));
    shiftLogs();
    logCount = MAX_LOGS - 1;
  }
  
  DateTime now = rtc.now();
  
  // Create log format: CardID|Name|Status|DD/MM/YY HH:MM
  String log = cardID + "|";
  if (name.length() > 0) {
    log += name;
  } else {
    log += "Unknown";
  }
  log += "|" + status + "|";
  log += String(now.day()) + "/" + String(now.month()) + "/" + String(now.year() % 100);
  log += " " + String(now.hour()) + ":" + String(now.minute());
  
  // Write log to EEPROM
  int addr = 100 + (logCount * LOG_SIZE);
  writeString(addr, log);
  
  // Update and save log count
  logCount++;
  writeInt(0, logCount);
  
  Serial.print(F("Logged: "));
  Serial.println(log);
}

// Display all logs via serial monitor
void viewLogs() {
  Serial.println(F("\n=== Attendance Logs ==="));
  Serial.print(F("Total: "));
  Serial.println(logCount);
  Serial.print(F("Inside: "));
  Serial.println(insideCount);
  Serial.println();
  
  if (logCount == 0) {
    Serial.println(F("No logs found."));
    return;
  }
  
  // Print header
  Serial.println(F("Card ID | Name | Status | Date Time"));
  Serial.println(F("--------|------|--------|----------"));
  
  // Print all log entries
  for (int i = 0; i < logCount; i++) {
    String log = readLogEntry(i);
    if (log.length() > 0) {
      // Parse and display log data
      int pos1 = log.indexOf('|');
      int pos2 = log.indexOf('|', pos1 + 1);
      int pos3 = log.indexOf('|', pos2 + 1);
      
      if (pos1 > 0 && pos2 > pos1 && pos3 > pos2) {
        String cardID = log.substring(0, pos1);
        String name = log.substring(pos1 + 1, pos2);
        String status = log.substring(pos2 + 1, pos3);
        String dateTime = log.substring(pos3 + 1);
        
        Serial.print(cardID);
        Serial.print(F(" | "));
        Serial.print(name);
        Serial.print(F(" | "));
        Serial.print(status);
        Serial.print(F(" | "));
        Serial.println(dateTime);
      }
    }
  }
}

// Display current system status via serial
void showStatus() {
  Serial.println(F("\n=== Current Status ==="));
  Serial.print(F("People inside: "));
  Serial.println(insideCount);
  Serial.print(F("Total logs: "));
  Serial.println(logCount);
  
  DateTime now = rtc.now();
  Serial.print(F("Current time: "));
  printDateTime(now);
  Serial.println();
}

// Clear all logs and reset counters
void clearLogs() {
  logCount = 0;
  insideCount = 0;
  writeInt(0, logCount);    // Save log count to EEPROM
  writeInt(2, insideCount); // Save inside count to EEPROM
  Serial.println(F("✓ All logs cleared!"));
}

// Shift all logs up by one position (remove oldest)
void shiftLogs() {
  for (int i = 1; i < logCount; i++) {
    String log = readLogEntry(i);
    int addr = 100 + ((i-1) * LOG_SIZE);
    writeString(addr, log);
  }
}

// Read specific log entry from EEPROM
String readLogEntry(int index) {
  int addr = 100 + (index * LOG_SIZE);
  return readString(addr, LOG_SIZE);
}

// Print formatted date and time to serial
void printDateTime(DateTime dt) {
  Serial.print(dt.day());
  Serial.print(F("/"));
  Serial.print(dt.month());
  Serial.print(F("/"));
  Serial.print(dt.year());
  Serial.print(F(" "));
  Serial.print(dt.hour());
  Serial.print(F(":"));
  if (dt.minute() < 10) Serial.print(F("0"));
  Serial.print(dt.minute());
}

// ===============================================
// EEPROM DATA STORAGE FUNCTIONS
// ===============================================

// Write single byte to EEPROM at specified address
void writeByte(int addr, byte data) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(addr >> 8);        // High byte of address
  Wire.write(addr & 0xFF);      // Low byte of address
  Wire.write(data);             // Data to write
  Wire.endTransmission();
  delay(5);                     // Write cycle delay
}

// Read single byte from EEPROM at specified address
byte readByte(int addr) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(addr >> 8);        // High byte of address
  Wire.write(addr & 0xFF);      // Low byte of address
  Wire.endTransmission();
  Wire.requestFrom(EEPROM_ADDR, 1);
  return Wire.available() ? Wire.read() : 0;
}

// Write string to EEPROM starting at specified address
void writeString(int addr, String str) {
  for (int i = 0; i < str.length() && i < LOG_SIZE-1; i++) {
    writeByte(addr + i, str[i]);
  }
  writeByte(addr + str.length(), 0);  // Null terminator
}

// Read string from EEPROM starting at specified address
String readString(int addr, int maxLen) {
  String result = "";
  for (int i = 0; i < maxLen-1; i++) {
    byte b = readByte(addr + i);
    if (b == 0) break;          // Stop at null terminator
    result += (char)b;
  }
  return result;
}

// Write 16-bit integer to EEPROM (uses 2 bytes)
void writeInt(int addr, int value) {
  writeByte(addr, value >> 8);      // High byte
  writeByte(addr + 1, value & 0xFF); // Low byte
}

// Read 16-bit integer from EEPROM (uses 2 bytes)
int readInt(int addr) {
  int high = readByte(addr);
  int low = readByte(addr + 1);
  return (high << 8) | low;
}

/*
 * ===============================================
 * END OF RFID ATTENDANCE SYSTEM CODE
 * ===============================================
 * 
 * USAGE INSTRUCTIONS:
 * 
 * 1. HARDWARE SETUP:
 *    - Connect all components as per pin definitions
 *    - Ensure proper power supply (5V for Arduino, 3.3V for RFID)
 *    - Use pull-up resistors for buttons if needed
 * 
 * 2. CARD PREPARATION:
 *    - Cards must be pre-programmed with Name and ID data
 *    - Use separate card programming sketch to write data
 *    - Name stored in blocks 4-6, ID in blocks 8-9
 * 
 * 3. OPERATION:
 *    - System shows current time and inside count on LCD
 *    - Scan valid RFID card to check IN/OUT
 *    - Use buttons to navigate menu system
 *    - View logs, status, and settings via LCD or Serial
 * 
 * 4. SERIAL COMMANDS:
 *    - 'v' - View all logs
 *    - 'c' - Clear all logs
 *    - 's' - Show current status
 *    - 't' - Set RTC time
 * 
 * 5. TROUBLESHOOTING:
 *    - Check all connections if system doesn't respond
 *    - Verify RFID cards are properly programmed
 *    - Use Serial Monitor at 9600 baud for debugging
 *    - Reset Arduino if LCD shows garbled text
 * 
 * FEATURES SUMMARY:
 * ✓ Automatic IN/OUT detection
 * ✓ Real-time logging with timestamps  
 * ✓ Persistent data storage in EEPROM
 * ✓ LCD menu navigation with buttons
 * ✓ Serial interface for remote monitoring
 * ✓ Inside count tracking
 * ✓ Text scrolling for long names
 * ✓ Access control for uninitialized cards
 * ✓ Log overflow protection
 * ✓ Button debouncing
 */