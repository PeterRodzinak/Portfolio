# Hardware & Embedded Software Engineering Portfolio

Welcome to my portfolio. My primary focus is on digital hardware design (RTL), low-level development, and embedded systems. I develop primarily in C++, design hardware using Verilog/SystemVerilog, and pragmatically use Python for fast prototyping and concept verification. 

Below is a selection of my most relevant projects.

---

## 1. Digital Design & RTL (Verilog / SystemVerilog)
Focused on raw hardware design and simulation.

* **RC6 encryption FPGA design**
  * **Tech stack:** Verilog, C++, Python
  * **Platform:** Basys3
  * **Description:** Design focused on basic AES-like encryption using a NIST-finalist algorithm RC6, using UART protocol for communication with PC. 
  * **Third-party modules:** RS232.v
  * 📂 [Link to source files and testbenches](./Projects/Hardware%20design/RC6_ENCRYPTION)

* **Diffie-Hellman key exchange on GF(2^79) Elicptic curves in FPGA**
  * **Tech stack:** Verilog, SystemVerilog, C++
  * **Platform:** Basys3
  * **Description:** Design of FPGA coprocessor for Diffie-Hellman key exchange over UART. Uses advanced optimizations like Itoh-Tsuji and Digital-Serial multiplication, benchmarked for less than 500 us per pure full exchange (does not include UART transceiving speed).
  * **Third-party modules:** RS232.v
  * 📂 [Link to source files and testbenches](./Projects/Hardware%20design/ECDH_KEY_EXCHANGE)

* **RSA digital signature in FPGA using 256b key**
  * **Tech stack:** SystemVerilog
  * **Platform:** Basys3
  * **Description:** Digital signature coprocessor over UART, using 256 bit RSA algorithm. Employs Montgomery reduction, Barrett reduction and Garner's algorithm to evade direct and slow/expensive division/modulo operations, and lowering chip space + increased speed using parallel CRT principle. As of the current state, it works on 50MHz.
  * **Third-party modules:** RS232.v
  * 📂 [Link to source files and testbenches](./Projects/Hardware%20design/RSA_DIGITAL_SIGNATURE)

## 2. Embedded C++ / Smartwatch OS (PlatformIO)
A comprehensive, scalable operating system and UI written for a microcontroller (LilyGo TWatch-S3 / ESP32), consisting of 1600+ lines of code.

* **Architecture:** Object-oriented design in modern C++ (inheritance, smart pointers) utilizing the LVGL graphics library.
* **Low-Level & Memory:** Features a **custom low-level BMP parser** (`utils.cpp`) that decodes images directly from the FFat file system without relying on external decoding libraries.
* **Power Management:** Custom sleep/low-power mode implementation, CPU underclocking, and wake-up management via hardware interrupts from the accelerometer and other sensors.
* **Networking:** Local REST client for smart home peripherals (JSON payload parsing and generation) and automated NTP time synchronization.
* 📂 [Link to source code (src) and architecture](./Projects/Embedded%20system%20application)

## 3. PCB Design & Hardware Routing (KiCad)
From schematic capture to final PCB layout.

* **Night lamp charged by photovoltaic panel with energy storage**
  * **Description:** Complete hardware design in KiCad 9.0, excluding only the photovoltaic panel and the accumulator battery. This folder contains project files, the schematic, and the completely routed PCB layout.
  * 🖼️ [3D render preview](./Projects/PCB%20design/PCB_3D.png)
  * 📄 [Vector schematic in PDF](./Projects/PCB%20design/Schema.pdf)
  * 📂 [Link to KiCad project files](./Projects/PCB%20design)