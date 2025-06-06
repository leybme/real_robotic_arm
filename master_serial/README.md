# Master Serial Controller

This Arduino sketch runs on an ESP32 and acts as a master controller for a robotic arm system. It reads ASCII commands from the USB Serial monitor and forwards them via ESP-NOW to up to three slave stepper controllers.

## Features

- **Serial Command Interface**: Read commands from USB Serial at 115200 baud
- **ESP-NOW Communication**: Wireless communication with up to 3 slave devices
- **Peer Management**: Register and manage multiple slave devices (R1, Z, R2)
- **Send Status Monitoring**: Track successful/failed message delivery
- **MAC Address Identification**: Display friendly names for each slave device

## Hardware Requirements

- ESP32 development board
- USB cable for Serial communication
- Slave ESP32 devices configured with matching MAC addresses

## Pin Configuration

No specific pins are used in this sketch - only Serial and ESP-NOW communication.

## Configuration

Before uploading, configure the MAC addresses of your slave devices:

```cpp
// R1 Slave Configuration
uint8_t R1_MAC[6] = {0xCC, 0xBA, 0x97, 0x60, 0x23, 0x24};
String R1_NAME = "R1";

// Z Slave Configuration  
uint8_t Z_MAC[6] = {0x94, 0xA9, 0x90, 0x43, 0xD0, 0xA0};
String Z_NAME = "Z";

// R2 Slave Configuration
uint8_t R2_MAC[6] = {0xCC, 0xBA, 0x97, 0x61, 0xD0, 0x0C};
String R2_NAME = "R2";
```

## Usage

1. **Upload the sketch** to your ESP32 master device
2. **Open Serial Monitor** at 115200 baud
3. **Type commands** and press Enter to send to slaves
4. **Monitor output** for send status and peer identification

### Example Commands

```
MOVE 100        # Move 100 steps relative
MOVETO 500      # Move to absolute position 500
SPEED 1000      # Set speed to 1000 steps/sec
ACCEL 500       # Set acceleration to 500 steps/sec²
SET0            # Zero current position
GRIP 90         # Set gripper to 90 degrees
ROTATE 45       # Rotate gripper to 45 degrees
```

## Key Functions

- **`setup()`**: Initializes WiFi, ESP-NOW, and registers slave peers
- **`loop()`**: Continuously reads Serial input and forwards commands
- **`sendMessage()`**: Sends ESP-NOW messages to specified peer
- **`onDataSent()`**: Callback for monitoring send status and peer identification

## Dependencies

- Arduino ESP32 Core
- WiFi library (included with ESP32 core)
- ESP-NOW library (included with ESP32 core)

## Notes

- Currently hardcoded to send all commands to R2_MAC slave
- To send to different slaves, modify the `sendMessage()` call in `loop()`
- ESP-NOW communication is unencrypted for simplicity
- Supports up to 250 byte messages per ESP-NOW packet

## Troubleshooting

- Ensure all devices are using the same WiFi channel
- Verify MAC addresses are correctly configured
- Check Serial Monitor for send status messages
- Restart devices if ESP-NOW initialization fails
