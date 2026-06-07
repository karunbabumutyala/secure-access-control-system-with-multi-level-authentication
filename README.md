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
* User Access Modes
* EEPROM Credential Storage
* UART Interrupt Communication
* Motorized Door Lock Control

---

# 🎯 Project Aim

To design and develop a secure embedded authentication system using fingerprint and password verification for restricted access applications with reliable user management and real-time status indication.

---

# 🧱 Block Diagram
<p align ="center">
<img width="1536" height="1024" alt="ChatGPT Image Jun 6, 2026, 10_38_32 PM" src="https://github.com/user-attachments/assets/b0ccff8a-48a7-4f15-9554-afb3031a5d21" />

</p>


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

# ⚡ Circuit Connections

| Component                  | LPC2148 Interface   |
| -------------------------- | ------------------- |
| ⌨️ 4x4 Matrix Keypad       | GPIO Pins           |
| 📟 LCD 16x2                | Data + Control Pins |
| 👆 R305 Fingerprint Sensor | UART0 Interface     |
| 💾 AT24C256 EEPROM         | I2C Interface       |
| ⚙️ L293D Motor Driver      | GPIO Output Pins    |
| 🔘 Switch                  | EINT0               |
| 🔋 Power Supply            | 3.3V / 5V           |


---

# 💻 Software Requirements

* Embedded C
* Keil uVision
* Flash Magic

---


# ✨ Features Implemented

* ✔ Multi-Level Authentication
* ✔ Fingerprint Enrollment & Verification
* ✔ EEPROM-Based Credential Storage
* ✔ UART Interrupt-Driven Communication
* ✔ Motorized Access Control
* ✔ LCD User Interface with Animation
* ✔ Retry & Timeout Handling
* ✔ Password-Based User Authentication
* ✔ Fingerprint Template Management
* ✔ EEPROM User Data Management

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

# 📸 Hardware Setup

<img width="1766" height="891" alt="image" src="https://github.com/user-attachments/assets/9b31b2e6-f3c8-4036-8c8a-52b99c7714c8" />


---

# 📟 LCD Outputs

## 🔓 Access Granted
<img width="1448" height="1086" alt="image" src="https://github.com/user-attachments/assets/b6a07240-440c-4c96-bfa7-b6cfb53c4ae9" />
---

## 🏠 Main Menu
<img width="623" height="357" alt="WhatsApp Image 2026-06-06 at 11 47 18 AM_edited" src="https://github.com/user-attachments/assets/b8640385-ec87-476b-9d8b-81b353fc9860" />

---

## 👆 Edit Menu

<img width="510" height="285" alt="WhatsApp Image 2026-06-06 at 11 46 06 AM_edited" src="https://github.com/user-attachments/assets/d9d0fc92-d963-439d-897c-b734672e53fd" />

---

# 👨‍💻 Author

Karun Babu Mutyala
Embedded Systems Enthusiast
# ⭐ GitHub Repository

If you like this project, give it a ⭐ on GitHub.

---

<div align="center">

## 💙 Thank You 💙

</div>
