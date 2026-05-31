# Hardware & Embedded Software Engineering Portfolio

Welcome to my portfolio. My primary focus is on digital hardware design (RTL), low-level development, and embedded systems. I develop primarily in C++, design hardware using Verilog/SystemVerilog, and pragmatically use Python for fast prototyping and concept verification. 

Below is a selection of my most relevant projects.

---

## 1. Digital Design & RTL (Verilog / SystemVerilog)
Focused on raw hardware design and simulation.

* **RC6 encryption FPGA design**
  * **Tech stack:** Verilog, C++, Python
  * **Description:** Design focused on basic AES-like encryption using a NIST-finalist algorithm RC6. 
  * 📂 [Link to source files and testbenches](./link-to-folder)

* **Diffie-Hellman key exchange on GF(2^79) Ellicptic curves in FPGA**
  * **Tech stack:** Verilog, SystemVerilog, Python
  * **Description:** [Add 1-2 sentences about what this hardware does, e.g., Bus architecture design / specific hardware module]. Includes custom source code and simulation testbenches. I use Python scripts to accelerate prototyping and verify complex logic prior to synthesis.
  * 📂 [Link to source files and testbenches](./link-to-folder)

* **RSA digital signature in FPGA using 256b key**
  * **Tech stack:** Verilog, SystemVerilog, Python
  * **Description:** [Add 1-2 sentences about what this hardware does, e.g., Bus architecture design / specific hardware module]. Includes custom source code and simulation testbenches. I use Python scripts to accelerate prototyping and verify complex logic prior to synthesis.
  * 📂 [Link to source files and testbenches](./link-to-folder)

## 2. Embedded C++ / Smartwatch OS (PlatformIO)
A comprehensive, scalable operating system and UI written for a microcontroller (LilyGo TWatch-S3 / ESP32), consisting of 1600+ lines of code.

* **Architecture:** Object-oriented design in modern C++ (inheritance, smart pointers) utilizing the LVGL graphics library.
* **Low-Level & Memory:** Features a **custom low-level BMP parser** (`utils.cpp`) that decodes images directly from the FFat file system without relying on external decoding libraries.
* **Power Management:** Custom sleep/low-power mode implementation, CPU underclocking, and wake-up management via hardware interrupts from the accelerometer and other sensors.
* **Networking:** Local REST client for smart home peripherals (JSON payload parsing and generation) and automated NTP time synchronization.
* 📂 [Link to source code (src) and architecture](./link-to-src)

## 3. PCB Design & Hardware Routing (KiCad)
From schematic capture to final PCB layout.

* **[Your Board Name, e.g., Solar Charge Controller / LED Driver]**
  * **Description:** Complete hardware design in KiCad 9.0. This folder contains project files, the schematic, and the completely routed PCB layout.
  * 🖼️ [3D render preview](./link-to-image.png)
  * 📄 [Vector schematic in PDF](./link-to-schematic.pdf)
  * 📂 [Link to KiCad project files](./link-to-kicad-folder)