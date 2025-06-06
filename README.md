# Real Robotic Arm Controller

This repository contains Arduino sketches for a master-slave robotic arm controller using ESP32 boards and ESP-NOW communication.

## Overview

- **Master (ESP32)** reads ASCII commands from the USB-Serial monitor and broadcasts them via ESP-NOW to one of up to three slave controllers.
- **Slave (ESP32)** listens for incoming ESP-NOW messages and drives stepper motors accordingly.

## Repository Structure

```
LICENSE
README.md
master_serial/
  └─ master_serial.ino
slave_codeR1/
  └─ slave_codeR1.ino
```

## Dependencies

- Arduino ESP32 core (install via Boards Manager)
- No additional libraries beyond `WiFi.h` and `esp_now.h`

## Configuration

1. **Master MAC addresses** (in `master_serial/master_serial.ino`):
   ```cpp
   uint8_t R1_MAC[6] = {0xCC, 0xBA, 0x97, 0x60, 0x23, 0x24};
   String  R1_NAME    = "R1";

   uint8_t Z_MAC[6]  = {0x94, 0xA9, 0x90, 0x43, 0xD0, 0xA0};
   String  Z_NAME     = "Z";

   uint8_t R2_MAC[6] = {0xCC, 0xBA, 0x97, 0x61, 0xD0, 0x0C};
   String  R2_NAME    = "R2";
   ```

## Usage

1. Flash the master and slave sketches to your ESP32 devices.
2. Open the Serial Monitor at **115200 baud** for the master.
3. Type commands (e.g., `MOVE 100`, `SPEED 500`, `SET0`) and press **Enter**.
4. Observe the send status and peer identification in the master’s log.

## License

This project is released under the [MIT License](LICENSE).
