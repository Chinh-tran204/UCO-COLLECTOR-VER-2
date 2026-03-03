# VinSchool Used Oil Collector Machine - Project Overview

This project implements an automated **Used Oil Collection Machine** designed for the "Eco Oil Coop" initiative. The machine accurately measures collected used oil, handles user authentication via a remote server, and manages power consumption for long-term deployment.

---

### 🛠 Hardware Architecture

The system is built on the **STM32F103C8T6 (Blue Pill)** microcontroller, utilizing its low-power capabilities and rich peripheral set.

| Component | Specification / Model | Function |
| :--- | :--- | :--- |
| **Microcontroller** | STM32F103C8T6 | Central processing and logic orchestration. |
| **Weight Sensor** | **HX711 + Load Cell** | Precision weight measurement of the collected oil. |
| **Level Sensor** | Ultrasonic (HC-SR04) | Non-contact volume measurement of the oil tank. |
| **Communication** | **SIMCOM A7680C** | 4G LTE Cat 1 communication for auth and data logging. |
| **Display** | **128x64 Monochrome LCD** | High-contrast status and data visualization (Graphic). |
| **User Input** | **12V Button with Red LED** | Industrial wake-up trigger and manual unlock status. |
| **Actuator** | Solenoid Lock | Secures the collection tank door. |
| **Power Mgmt** | Internal RTC + ADC | Battery monitoring and timed wake-up cycles. |

---

### ⚙️ Core Functionality

1.  **Dual-Sensor Monitoring:** 
    *   **Volume:** Uses ultrasonic waves to calculate distance to the oil surface.
    *   **Weight:** Utilizes the **HX711** 24-bit ADC for high-precision weight tracking.
2.  **Remote Authentication:** To unlock the machine, the user holds the button for 3 seconds. The system requests authorization from the central server via the **A7680C** module using a secure machine ID.
3.  **Cloud Data Logging:** Post-collection, the machine automatically transmits the current oil volume, weight, and battery health to the `bi-oil.app` backend.
4.  **Energy Efficiency:** The machine spends most of its time in **STOP Mode** (ultra-low power). It wakes up via:
    *   **12V Physical Button press:** When a user arrives.
    *   **RTC Alarm:** For scheduled status updates.
5.  **Localized UI:** Optimized for the **128x64 Monochrome screen**, providing clear instructions in both Vietnamese and English.

---

### 📂 Software Structure

The firmware is developed using the **STM32 HAL** framework and structured into modular drivers:

*   `Core/Src/main.c`: Primary State Machine logic.
*   `Core/Src/UI.c`: UI rendering logic for multiple LCD types.
*   `Core/Src/SIMCOM.c`: AT-command based HTTP client for GSM communication.
*   `Core/Src/Ultrasonic.c`: High-precision timing for distance calculation.
*   `Drivers/csrc/`: Contains the **U8g2** library for graphic LCD control.

---

### 🚀 Getting Started

#### 1. Hardware Preparation
*   Ensure the SIM card is inserted and has an active data plan (GPRS).
*   Connect the Ultrasonic sensor to `PB4` (Echo) and `PB5` (Trig).
*   Connect the Solenoid Lock to `PA6` via a relay or MOSFET driver.
*   Verify battery voltage is within the expected range for the ADC divider on `PA1`.

#### 2. Firmware Installation
1.  Open the project in **STM32CubeIDE**.
2.  Adjust tank dimensions (`MAX_HEIGHT`, `MIN_HEIGHT`, `AREA`) in `Core/Src/main.c` if using a custom container.
3.  Compile and flash the firmware using an ST-Link v2 debugger.

#### 3. Operation Workflow
*   **Idle:** The machine's LED/LCD will be off to save power.
*   **Activation:** Press the main button to wake the system.
*   **Measuring:** The screen will display the current volume and weight.
*   **Unlocking:** Hold the button for **3 seconds**. The machine will verify with the server. If authorized, the buzzer sounds and the door unlocks.
*   **Finalize:** After the door closes, the system updates the server and returns to sleep.

---

### 📝 Development Notes
*   **API Endpoints:**
    *   `GET`: `https://api.admin.bi-oil.app/vinschool-machine/get-auth`
    *   `POST`: `https://api.admin.bi-oil.app/postcontainer/`
*   **Debug Port:** Serial log available via **USART1** (115200 baud).

---

> **Senior Project Manager Note:** 
> *Ensure the machine's ultrasonic sensor is shielded from foam or turbulence during oil disposal to maintain measurement accuracy. Battery calibration should be performed per unit to ensure accurate power reporting.*

---
*Created for Vinschool Sustainability Project - 2026*
