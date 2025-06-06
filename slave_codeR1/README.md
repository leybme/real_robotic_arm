# Stepper Slave Controller (R1)

This Arduino sketch runs on an ESP32 and acts as a slave controller for stepper motors in a robotic arm system. It receives ASCII commands via ESP-NOW or USB Serial and controls stepper motors using the AccelStepper library, with optional gripper servo control.

## Features

- **Dual Command Input**: Accepts commands via ESP-NOW wireless or USB Serial
- **AccelStepper Integration**: Smooth acceleration/deceleration control
- **EEPROM Persistence**: Saves position, speed, and acceleration settings
- **Servo Gripper Control**: Optional gripper and rotation servo support
- **Non-blocking Operation**: Continuous stepper movement without blocking
- **Comprehensive Commands**: Move, speed, acceleration, and gripper control

## Hardware Requirements

- ESP32 development board
- Stepper motor with step/direction driver (e.g., A4988, DRV8825)
- Optional: 2x servo motors for gripper control
- Power supply appropriate for your stepper motor

## Pin Configuration

```cpp
#define PIN_STEP_EN 0    // Enable pin (LOW = enabled)
#define PIN_STEP    1    // STEP pulse pin
#define PIN_DIR     4    // DIRECTION pin
#define PIN_GRIP    2    // Gripper servo pin (optional)
#define PIN_GRIP_ROTATE  3    // Gripper rotation servo pin (optional)
```

**Note**: Adjust these pin definitions to match your wiring setup.

## Supported Commands

| Command | Format | Description | Example |
|---------|--------|-------------|---------|
| `MOVE` | `MOVE <steps>` | Relative move by specified steps | `MOVE 100` |
| `MOVETO` | `MOVETO <position>` | Absolute move to position | `MOVETO 500` |
| `SPEED` | `SPEED <steps/sec>` | Set maximum speed | `SPEED 1000` |
| `ACCEL` | `ACCEL <steps/sec²>` | Set acceleration | `ACCEL 500` |
| `SET0` | `SET0` | Zero current position | `SET0` |
| `GRIP` | `GRIP <angle>` | Set gripper servo angle (0-180°) | `GRIP 90` |
| `ROTATE` | `ROTATE <angle>` | Set rotation servo angle (0-180°) | `ROTATE 45` |

## Configuration

### Default Settings
- **Speed**: 8000 steps/sec
- **Acceleration**: 2500 steps/sec²
- **Initial Position**: 0 (loaded from EEPROM)

### EEPROM Storage
Settings are automatically saved to EEPROM and restored on startup:
- Position (long, 4 bytes)
- Speed (long, 4 bytes) 
- Acceleration (long, 4 bytes)

## Usage

### Via ESP-NOW (Wireless)
1. Configure master device with this slave's MAC address
2. Send commands from master controller
3. Monitor Serial output for command confirmation

### Via USB Serial
1. Open Serial Monitor at 115200 baud
2. Type commands directly and press Enter
3. View real-time feedback and status

## Key Functions

- **`setup()`**: Initializes hardware, EEPROM, WiFi, and ESP-NOW
- **`loop()`**: Handles Serial input and runs stepper motor
- **`executeCommand()`**: Parses and executes received commands
- **`onEspNowReceive()`**: ESP-NOW message receive callback

## Dependencies

- **Arduino ESP32 Core**
- **AccelStepper Library** (install via Library Manager)
- **ESP32Servo Library** (install via Library Manager)
- **WiFi Library** (included with ESP32 core)
- **ESP-NOW Library** (included with ESP32 core)
- **EEPROM Library** (included with ESP32 core)

## Installation

1. Install required libraries through Arduino IDE Library Manager
2. Configure pin definitions to match your hardware
3. Upload sketch to ESP32 slave device
4. Note the MAC address displayed in Serial Monitor
5. Configure master device with this MAC address

## Example Wiring

### Stepper Driver (A4988/DRV8825)
```
ESP32 Pin 0  → ENABLE (or leave floating for always-on)
ESP32 Pin 1  → STEP
ESP32 Pin 4  → DIR
ESP32 GND    → GND
ESP32 3.3V   → VDD (logic power)
```

### Servo Motors (Optional)
```
ESP32 Pin 2  → Gripper servo signal
ESP32 Pin 3  → Rotation servo signal
ESP32 GND    → Servo GND
ESP32 5V     → Servo VCC (or external 5V supply)
```

## Troubleshooting

- **Stepper not moving**: Check enable pin (should be LOW), verify wiring
- **Commands not received**: Verify MAC address configuration on master
- **EEPROM issues**: Check for "EEPROM was empty" message, defaults will be set
- **Servo not responding**: Ensure ESP32Servo library is installed and pins are correct
- **ESP-NOW init failed**: Restart device, check WiFi configuration

## Notes

- EEPROM settings persist across power cycles
- Stepper runs continuously in non-blocking mode
- Enable pin is set LOW (active) by default
- Gripper features can be disabled by commenting out `#define GRIP`
