# 🔐 Secure Access Control System with Multi-Level Authentication using LPC2148

---

# 📌 Project Overview

This project implements a Secure Access Control System using the LPC2148 ARM7 Microcontroller with biometric and password-based authentication.

The system is designed to provide secure access using the R305 fingerprint sensor, EEPROM credential management, keypad-based password entry, and motorized door locking mechanism.

It supports:

* Fingerprint Enrollment
* Fingerprint Verification
* Fingerprint Deletion
* Password Authentication
* Admin/User Access Modes
* EEPROM Credential Storage
* UART Interrupt Communication
* Motorized Door Lock Control

---

# 🎯 Project Aim

To design and develop a secure embedded authentication system using fingerprint and password verification for restricted access applications with reliable user management and real-time status indication.

---

# 🧱 Block Diagram

(Add your block diagram image here)

```text
Fingerprint Sensor  ---> LPC2148 ---> LCD
Keypad              ---> LPC2148 ---> EEPROM
L293D Motor Driver  ---> Door Lock Mechanism
```

---

# ⚙️ System Workflow

## 🔹 Enrollment

* Capture fingerprint image
* Generate fingerprint template
* Store fingerprint in R305 memory
* Store ID and password in EEPROM

## 🔹 Verification

* User enters ID/password
* Fingerprint matching performed
* Access granted or denied

## 🔹 Deletion

* Admin enters user ID
* Fingerprint template deleted
* EEPROM credentials removed

---

# 🛠️ Hardware Requirements

| Component  | Description          |
| ---------- | -------------------- |
| LPC2148    | ARM7 Microcontroller |
| R305       | Fingerprint Sensor   |
| AT24C256   | EEPROM               |
| 16x2 LCD   | Display Unit         |
| 4x4 Keypad | Password Input       |
| L293D      | Motor Driver         |
| DC Motor   | Door Lock Control    |
| Buzzer     | Alert System         |

---

# 💻 Software Requirements

* Embedded C
* Keil uVision
* Flash Magic

---

# 📂 Repository Structure

```text
Secure-Access-Control-System/
│
├── main.c
├── r305.c
├── lcd.c
├── uart.c
├── i2c.c
├── kpm.c
├── delay.c
│
├── Drivers/
│   ├── LCD
│   ├── UART
│   ├── EEPROM
│   └── Fingerprint
│
├── Images/
├── Documents/
└── README.md
```

---

# ✨ Features Implemented

✔ Multi-Level Authentication
✔ Fingerprint Enrollment & Verification
✔ EEPROM-Based Credential Storage
✔ UART Interrupt-Driven Communication
✔ Motorized Access Control
✔ LCD User Interface with Animation
✔ Retry & Timeout Handling

---

# ✅ Advantages

* Secure biometric authentication
* Reliable password validation
* Efficient EEPROM memory management
* Real-time LCD status indication
* Low-cost embedded security solution
* Easy user management
* Expandable system architecture

---

# 📸 Project Output

(Add hardware setup and LCD output images here)

---

# 👨‍💻 Author

Karun Babu Mutyala
Embedded Systems Enthusiast
