RFID Based Attendance System Using Arduino
==========================================

[![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=Arduino&logoColor=white)](https://www.arduino.cc/) [![C++](https://img.shields.io/badge/C%2B%2B-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)](https://isocpp.org/) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](https://opensource.org/licenses/MIT) [![CircuitDigest](https://img.shields.io/badge/Tutorial-CircuitDigest-blue?style=for-the-badge)](https://circuitdigest.com/microcontroller-projects/rfid-based-attendance-system-project-using-arduino)

A complete **RFID based attendance system using Arduino** that automates employee and student attendance tracking with real-time logging, LCD display, and offline data storage capabilities.

![RFID Attendance System](https://circuitdigest.com/sites/default/files/inlineimages/u5/RFID-Attendance-System-Using-Arduino.png)

🚀 Features
-----------

-   **Automatic Check-in/Check-out Detection** - Smart status tracking based on last entry
-   **Real-time Clock Integration** - Precise timestamps with DS1307 RTC module
-   **Offline Data Storage** - 30+ attendance logs stored in EEPROM memory
-   **LCD Interface** - 16x2 I2C display with scrolling names and live people count
-   **Card Programming Utility** - Easy RFID card setup with personal data
-   **Serial Monitor Control** - Full system control via Arduino IDE
-   **Power Loss Protection** - Data retention during power outages
-   **Cost-Effective** - Under $35 total build cost vs $500+ commercial systems


🛠️ Hardware Requirements
-------------------------

### Primary Components

-   **Arduino Uno** (1x) - ATmega328P microcontroller board
-   **MFRC522 RFID Module** (1x) - 13.56MHz RFID reader with SPI interface
-   **DS1307 RTC Module** (1x) - Real-time clock with AT24C32 EEPROM
-   **16x2 I2C LCD Display** (1x) - HD44780 compatible with I2C backpack
-   **Push Buttons** (2x) - 6x6mm tactile buttons for navigation
-   **RFID Cards/Tags** - Mifare Classic 1K, 13.56MHz
-   **Breadboard** (1x) - Full-size for prototyping
-   **Jumper Wires** - Male-to-male connections
-   **Power Supply** - 5V-12V DC adapter or battery

### Optional Components

-   **Buzzer** - Audio feedback for card scans
-   **LED Indicators** - Visual status indicators
-   **SD Card Module** - Additional storage backup
-   **Ethernet Shield** - Network connectivity

📐 Circuit Diagram
------------------

```
Arduino Uno Connections:
┌─────────────────┬──────────────────┬────────────────┐
│ Arduino Pin     │ Component        │ Function       │
├─────────────────┼──────────────────┼────────────────┤
│ Digital Pin 7   │ MFRC522 RST      │ RFID Reset     │
│ Digital Pin 8   │ Menu Button      │ Navigation     │
│ Digital Pin 9   │ Select Button    │ Confirmation   │
│ Digital Pin 10  │ MFRC522 SS       │ SPI Slave Sel. │
│ Digital Pin 11  │ MFRC522 MOSI     │ SPI Data Out   │
│ Digital Pin 12  │ MFRC522 MISO     │ SPI Data In    │
│ Digital Pin 13  │ MFRC522 SCK      │ SPI Clock      │
│ Analog Pin A4   │ SDA              │ I2C Data       │
│ Analog Pin A5   │ SCL              │ I2C Clock      │
└─────────────────┴──────────────────┴────────────────┘

```

**📖 Complete Circuit Diagram:** [View Detailed Schematic](https://circuitdigest.com/microcontroller-projects/rfid-based-attendance-system-project-using-arduino)

⚙️ Installation
---------------

### 1\. Arduino IDE Setup

```
# Install required libraries via Arduino Library Manager:
# - MFRC522 by GithubCommunity
# - RTClib by Adafruit
# - LiquidCrystal I2C by Frank de Brabander

```

### 2\. Hardware Assembly

1.  Connect components according to circuit diagram
2.  Ensure proper power distribution (5V/3.3V compatibility)
3.  Verify I2C addresses (LCD: 0x27, RTC: 0x68, EEPROM: 0x50)

### 3\. Code Upload

```
git clone https://github.com/Circuit-Digest/RFID-Based-Attendance-System-Using-Arduino.git
cd RFID-Based-Attendance-System-Using-Arduino

```

1.  **First**: Upload `ReadandWrite_PersonalData.ino` to program RFID cards
2.  **Second**: Upload `RFIDAttendanceSystem_Main.ino` for main operation

🎯 Usage
--------

### Programming RFID Cards

1.  Upload the card programming sketch
2.  Open Serial Monitor (9600 baud)
3.  Type `w` for write mode
4.  Place RFID card near reader
5.  Enter name (max 32 characters)
6.  Enter ID (max 8 characters)
7.  Verify with read mode (`r`)

### Operating the Attendance System

1.  Upload main attendance code
2.  Set correct time via Serial Monitor (`t` command)
3.  Present programmed RFID cards to reader
4.  System automatically tracks IN/OUT status
5.  View logs via buttons or Serial Monitor commands

### Serial Commands

-   `v` - View all attendance logs
-   `c` - Clear all logs
-   `s` - System status
-   `t` - Set time

📁 Code Structure
-----------------

```
├── Code/
│   ├── RFIDAttendanceSystem_Main/
│   │   └── RFIDAttendanceSystem_Main.ino    # Main attendance system
│   └── ReadandWrite_PersonalData/
│       └── ReadandWrite_PersonalData.ino    # Card programming utility
├── Circuit Diagram/
│   └── RFID_Attendance_Circuit.png          # Wiring schematic
├── Documentation/
│   └── Component_Connections.md             # Detailed connections
└── README.md                                # This file

```

### Key Functions

-   `handleRFIDScan()` - Processes card detection and logging
-   `handleButtons()` - Manages menu navigation
-   `displayHomeScreen()` - Updates LCD with time/status
-   `logAttendance()` - Stores records in EEPROM
-   `readPersonalData()` - Retrieves card information

🔧 Troubleshooting
------------------

### Common Issues

**RFID Cards Not Detected**

-   Check SPI connections (pins 10-13)
-   Verify 3.3V power to MFRC522
-   Try different RFID cards (Mifare Classic 1K)

**LCD Display Issues**

-   Scan I2C address (may be 0x3F instead of 0x27)
-   Check I2C connections (A4/A5)
-   Adjust contrast potentiometer

**Time Resets on Power Loss**

-   Replace CR2032 battery in RTC module
-   Check RTC module connections

**Data Loss Issues**

-   Verify EEPROM connections
-   Check AT24C32 module functionality

📱 Applications
---------------

### Educational Institutions

-   **Student Attendance** - Automated classroom tracking
-   **Library Access** - Book borrowing management
-   **Hostel Entry** - Secure dormitory access

### Corporate Environments

-   **Employee Time Tracking** - Work hour monitoring
-   **Meeting Room Access** - Conference room management
-   **Visitor Management** - Guest entry logging

### Industrial Settings

-   **Shift Management** - Worker schedule tracking
-   **Safety Compliance** - Restricted area monitoring
-   **Contractor Access** - Temporary worker management

🔮 Future Enhancements
----------------------

-   [ ] **WiFi Connectivity** - ESP8266 integration for IoT capabilities
-   [ ] **Mobile App** - Android/iOS app for remote monitoring
-   [ ] **Biometric Security** - Fingerprint sensor integration
-   [ ] **Camera Module** - Photo capture for verification
-   [ ] **Database Integration** - MySQL/MongoDB connectivity
-   [ ] **Multi-Location** - Network multiple units
-   [ ] **Access Control** - Door lock integration
-   [ ] **Web Dashboard** - Real-time monitoring interface

🏗️ Technical Specifications
----------------------------

| Parameter | Value |
| --- | --- |
| Operating Voltage | 5V DC |
| Current Consumption | 150-200mA (active) |
| RFID Frequency | 13.56MHz |
| Reading Range | 1-3cm |
| Storage Capacity | 30 logs (expandable) |
| Clock Accuracy | ±2 minutes/month |
| Operating Temperature | 0°C to 50°C |
| Response Time | <500ms |

🤝 Contributing
---------------

We welcome contributions! Please follow these steps:

1.  Fork the repository
2.  Create a feature branch (`git checkout -b feature/AmazingFeature`)
3.  Commit changes (`git commit -m 'Add AmazingFeature'`)
4.  Push to branch (`git push origin feature/AmazingFeature`)
5.  Open a Pull Request

### Contributing Guidelines

-   Follow Arduino coding standards
-   Test all modifications thoroughly
-   Update documentation for new features
-   Include comments for complex code sections


🔗 Links
--------

-   **📖 Complete Tutorial**: [CircuitDigest - RFID Attendance System Guide](https://circuitdigest.com/microcontroller-projects/rfid-based-attendance-system-project-using-arduino)
-   **📚 More Arduino Projects**: [Circuit Digest Arduino Collection](https://circuitdigest.com/arduino-projects)
-   **🎓 RFID Tutorials**: [RFID Projects and Guides](https://circuitdigest.com/rfid-projects)

⭐ Support
---------

If this project helped you, please ⭐ **star this repository** and share it with others!


* * * * *

**Built with ❤️ by [Circuit Digest](https://circuitdigest.com/) | Making Electronics Simple**

* * * * *

### Keywords

`arduino rfid attendance system` `rfid based attendance tracker` `diy attendance system` `arduino rfid project` `employee attendance system` `student attendance tracker` `rfid scanner arduino` `attendance management system` `arduino projects` `rfid applications`
