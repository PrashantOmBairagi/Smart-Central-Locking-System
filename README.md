
A smart, flexible central locking system for Alto 800
# Smart Car Central Locking System for Alto 800

> *A fully programmable, low‑power, and modular central locking system built from scratch — no comprehensive references existed online.*

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Key Features](#key-features)
3. [Hardware Components & Technical Rationale](#hardware-components--technical-rationale)
4. [Power Consumption Calculations](#power-consumption-calculations)
5. [Software Architecture](#software-architecture)
6. [Wiring Diagram & Schematics](#wiring-diagram--schematics)
7. [Folder Structure](#folder-structure)
8. [Installation & Usage](#installation--usage)
9. [Development Notes & Learnings](#development-notes--learnings)
10. [Future Work](#future-work)
11. [License](#license)
12. [Author](#author)

---

## Project Overview

Building a custom central locking system for my Alto 800 presented unique challenges:

* **No complete open‑source design**: Only a single actuator pinout image on Pinterest, no end‑to‑end wiring or logic workflows.
* **Resource constraints**: Worked from a rural home setup with intermittent internet and delayed component delivery.
* **Zero mentorship**: Reverse‑engineered RF modules, actuators, and analog circuits without official datasheets.

By combining Arduino-based control, RF communication (433 MHz), analog comparators, and actuator drivers, this project demonstrates true hardware‑software co‑design and perseverance.

---

## Key Features

* **Programmable behavior**: Change lock/unlock logic with code—no fixed firmware.
* **Smart auto-lock**: Prevents locking when ignition on or doors open.
* **Ultra-low power**: Sleep modes / 50% Duty-Cycle reduce standby draw to \~5 mA.
* **Modular & expandable**: Supports rolling codes, sensors, GSM modules.
* **Compact design**: Future custom PCB mimics car ECU form factor.

---

## Hardware Components & Technical Rationale

| Component                    | Specification                                   | Rationale                                                                                                          |
| ---------------------------- | ----------------------------------------------- | ------------------------------------------------------------------------------------------------------------------ |
| **Arduino Pro Mini (3.3 V)** | ATmega328P @8 MHz, 3.3 V                        | Chosen for low-power operation and compatibility with RF module; operates stably on 3.3 V from LM2596 buck output. |
| **Buck Converter (LM2596)**  | Adjustable 3.3 V & 12 V output, 2 A             | Steps down 12 V car battery to stable 3.3 V for logic circuits; also supplies 12 V rail to actuators and buzzer.   |
| **H‑Bridge Driver (L298N)**  | Dual DC motor driver, 2 A/channel, 46 V max     | Drives two 12 V door actuators with bidirectional control; integrates protection diodes and 5 V logic interface.   |
| **RF 433 MHz Receiver**      | ASK/OOK, 3.3 V logic compatible                 | Directly compatible with 3.3 V system; receives lock/unlock signals and outputs data to comparator.                |
| **Op‑Amp (LM358)**           | Dual-channel, single-supply operation           | Configured as comparator to clean noisy RF output, provide threshold control and analog hysteresis.                |
| **Door Position Sensor**     | SPDT magnetic reed switch, dry contact          | Detects door open/closed condition; connected directly to digital I/O on Pro Mini for intelligent locking logic.   |
| **12 V Buzzer**              | Piezo buzzer, powered from 12 V rail            | Provides user feedback for locking actions; driven via transistor or MOSFET from logic output.                     |
| **Actuators (Front & Rear)** | 12 V DC, \~300 mA stall current                 | Stock central-lock actuators; controlled via L298N channels for isolated, bidirectional operation.                 |
| **Flyback Diodes & Caps**    | 1 N4007 across coils, 0.1 µF & 10 µF capacitors | Suppress back-EMF and voltage spikes; ensure clean operation of both digital and power sections.                   |

\----------------------------- | ------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------- |
\| **Arduino Pro Mini (5 V)**    | ATmega328P @16 MHz, 5 V                          | Chosen for direct compatibility with 5 V from LM2596 buck converter; sufficient GPIO for RF, comparator, sensors, and H-bridge control. |
\| **Buck Converter (LM2596)**   | Adjustable 3.3 V & 5 V output, 2 A                 | Steps down 12 V car battery to stable 5 V for Pro Mini and 3.3 V for RF module; handles inrush current with onboard 10 A fuse.    |
\| **H‑Bridge Driver (L298N)**   | Dual DC motor driver, 2 A/channel, 46 V max      | Drives two 12 V door actuators with bidirectional control; built‑in protection diodes reduce external components.            |
\| **RF 433 MHz Receiver**       | ASK/OOK, 8 pins (VCC, GND, DATA×2, etc.)         | Simple interface; powered at 3.3 V for noise immunity; decoder connected to Pro Mini digital pin via comparator.             |
\| **Op‑Amp (LM358)**            | Dual, rail-to-rail, low-cost                     | Configured as comparator to clean RX data, implement hysteresis, and filter analog noise before microcontroller input.     |
\| **Door Position Sensor**      | SPDT magnetic reed switch, 12 V compatible       | Detects door open/closed state; wired to Pro Mini digital input for smart auto-lock inhibition.                           |
\| **12 V Buzzer**               | 12 V piezoelectric buzzer                         | Provides audible feedback on lock/unlock events; driven via MOSFET controlled by Pro Mini digital output.                   |
\| **Actuators (Front & Rear)**  | 12 V DC, \~300 mA stall current                    | Stock central-lock actuators; wired through L298N channels; current-limiting inherent to driver.                           |
\| **Voltage Regulator (3.3 V)** | LM2596 output channel                             | Powers RF module at 3.3 V for reliable digital signal decoding and reduced EMI.                                            |
\| **Flyback Diodes & Caps**     | 0.1 µF & 10 µF decoupling; 1 N4007 diodes         | Decoupling caps on all power rails to suppress spikes; diodes across actuator coils to protect against back-EMF.            |

\------------------------------- | --------------------------------- | ------------------------------------------------------------------------------------------------------------------------ |
\| **Arduino Pro Mini (3.3V)**     | ATmega328P @8 MHz, 3.3V           | Compact footprint, low active (\~20mA) & sleep (\~2 mA) draw; common community support.                                  |
\| **RF 433 MHz TX/RX modules**    | ASK/OOK, simple two-pin interface | Low cost, sufficient range (\~10 m), easy Arduino integration; reverse‑engineered pins.                                  |
\| **Op‑Amp (LM358)**              | Dual, rail-to-rail, low-cost      | Chosen for its ability to operate near supply rails; used in comparator config to clean noisy RF signal.                 |
\|                                 |                                   |                                                                                                                          |
\| **Actuators**                   | 12 V DC, \~300 mA stall current   | Stock central-lock actuators; tested stall current with multimeter; incorporated current-limiting and transistor driver. |
\| **MOSFET (IRLZ44N)**            | Logic-level gate, low RDS(on)     | Efficient switching of 12 V to actuators; low heat dissipation in automotive environment.                                |
\| **Voltage Regulator (AMS1117)** | 3.3 V output, 1 A max             | Stabilizes 12 V car battery to Arduino’s 3.3 V rail; required small decoupling caps to suppress EMI.                     |
\| **Bypass & Filter Caps**        | 0.1 μF & 10 μF decoupling         | Essential to reduce voltage spikes and filter noise from inductive loads.                                                |

---

## Power Consumption Calculations

Assumptions:

* Car battery: 12 V, 35 Ah (\~420 Wh)
* System logic voltage: 3.3 V (Arduino Pro Mini, RF, comparator)
* Average lock/unlock event: 0.5 s actuator drive at 300 mA
* Typical use: 10 events/day

| Mode               | Current Draw    | Duty Cycle      | Energy/Day                                                |
| ------------------ | --------------- | --------------- | --------------------------------------------------------- |
| **Deep Sleep**     | \~1.2 mA @3.3 V | 99.99%          | 1.2 mA × 24 h = 28.8 mAh (\~0.095 Wh)                     |
| **Active MCU**     | \~10 mA @3.3 V  | 0.01%           | 10 mA × (0.5 s × 10)/3600 s = \~0.014 mAh (\~0.000046 Wh) |
| **Actuator Drive** | 300 mA @12 V    | \~5 s/day total | 300 mA × (0.5 s × 10)/3600 s = \~0.417 mAh (\~0.005 Wh)   |

**Total Daily Energy Consumption:** \~0.1 Wh/day (extremely low compared to 420 Wh car battery capacity)

**Total Energy/Day:** \~0.245 Wh/day (negligible vs 420 Wh battery)
**In my opinion**, this ultra-low average consumption ensures the car battery remains unaffected even after weeks of inactivity.

---

## Software Architecture

1. **RF Decoder**: Samples RX module, debounces via hysteresis, validates packet format.
2. **LockController**: Drives MOSFETs to actuators based on decoded commands and sensor inputs.
3. **Power Manager**: Enters 50% Duty-Cycle between signals; wakes via pin-change or timer.

---

## Wiring Diagram & Schematics

**Key Points:**

* RF RX data pin t → digital pin on Pro Mini
* Pro Mini digital 7/8→ Motor Driver A1/A2 → actuator;
* Power decoupled with 10 μF & 0.1 μF caps

---

## Folder Structure

```
Smart-Central-Locking-System/
├── code/
│   └── central_locking.ino
├── media/
│   ├── actuator_test.jpg
│   ├── in_car_demo.mp4
│   └── breadboard_closeup.png
├── Schematic_Central-Lock_2025-07-13.pdf
│
└── README.md
```

---

## Installation & Usage

1. Clone the repository:

   ```bash
   git clone https://github.com/<username>/Smart-Central-Locking-System.git
   cd Smart-Central-Locking-System/code
   ```
2. Open `central_locking.ino` in Arduino IDE (>=1.8.x).
3. Install required libraries: `LowPower` (optional).
4. Update `config.h` with your pin mappings.
5. Upload to Arduino Pro Mini at 5 V/16 MHz.
6. Assemble circuit per wiring diagram.
7. Test lock/unlock via your RF remote.

---

## Development Notes & Learnings

* **Reverse Engineering**: Identified RF module pins by multimeter continuity tests—no datasheet available.
* **Analog Noise**: Implemented RC hysteresis around LM358 to stabilize chatter from inductive loads.
* **Component Failures**: Burnt 3 Pro Mini boards; optimized gate resistors and added flyback diodes.
* **Automotive Environment**: Learned about voltage spikes and EMI mitigation in real vehicles.

---

## Future Work

* **Custom PCB**: Compact board replicating this breadboard logic with automotive-grade components.
* **Security**: Implement rolling-code algorithm (e.g., KeeLoq) on ATtiny85.
* **Sensor Integration**: Add seat-occupancy or door-open sensors for smarter auto-lock logic.
* **IoT Extension**: GSM/GPRS module for remote control or status notifications.

---

## License

Free to Use 

---

## Author

**Prashant Bairagi**
Electronics Engineering Student, Jabalpur Engineering College (RGPV Bhopal)
LinkedIn: [https://www.linkedin.com/in/](www.linkedin.com/in/prashant-bairagi-kmlpr)

*Built with curiosity, persistence, and a will to learn.*
