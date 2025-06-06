/*
  Stepper Slave Controller
  ========================
  - Receives ASCII commands over ESP-NOW or USB CDC (Serial)
  - Uses AccelStepper to drive a step/dir stepper driver
  - Supported commands:
      MOVE XX     : relative move by XX steps
      MOVETO XX   : absolute move to position XX (steps)
      SPEED XX    : set maximum speed to XX (steps/sec)
      ACCEL XX    : set acceleration to XX (steps/sec²)
      SET0        : zero the current position (set position = 0)
*/

#include <WiFi.h>
#include <esp_now.h>
#include <AccelStepper.h>
#include <EEPROM.h>    // for persistent storage
#define EEPROM_SIZE 64 // bytes for position, speed, accel
// Servo control for gripper (ESP32-compatible)
#include <ESP32Servo.h>

// Pin definitions (adjust to your wiring)
#define PIN_STEP_EN 0 // Enable pin (LOW = enabled)
#define PIN_STEP 1    // STEP pin
#define PIN_DIR 4     // DIR pin
#define GRIP
#ifdef GRIP
#define PIN_GRIP 2        // GRIP pin (optional, for gripper control)
#define PIN_GRIP_ROTATE 3 // GRIP_ROTATE pin (optional, for gripper rotation)
#endif

// AccelStepper: DRIVER mode (Step + Dir)
AccelStepper stepper(AccelStepper::DRIVER, PIN_STEP, PIN_DIR);

// Servo objects
Servo servoGrip;
Servo servoRotate;

// Stored settings
long savedPosition = 0;
long savedSpeed = 8000;
long savedAccel = 2500;

// Correct ESP-NOW receive callback signature:
void onEspNowReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
  // data is ASCII command, e.g. "MOVE 100\n"
  String cmd = String((char *)data).substring(0, len);
  cmd.trim();
  if (cmd.length() > 0)
  {
    executeCommand(cmd);
  }
}

// ----------------------------------------------------------------------------
// ---- Command parser & executor -----------------------------------------------
void executeCommand(const String &cmd)
{
  // Split at first space
  int sp = cmd.indexOf(' ');
  String key = (sp == -1) ? cmd : cmd.substring(0, sp);
  key.toUpperCase();

  String arg = (sp == -1) ? "" : cmd.substring(sp + 1);
  long value = 0;
  if (arg.length() > 0)
  {
    value = arg.toInt();
  }

  if (key == "MOVE")
  {
    // Relative move: current position + value
    stepper.move(stepper.currentPosition() + value - stepper.currentPosition());
    Serial.printf("CMD: MOVE %ld -> moving to %ld\n", value, stepper.targetPosition());
    // save new target
    savedPosition = stepper.targetPosition();
    EEPROM.put(0, savedPosition);
    EEPROM.commit();
    Serial.printf("Saved position to EEPROM: %ld\n", savedPosition);
  }
  else if (key == "MOVETO")
  {
    // Absolute move
    stepper.moveTo(value);
    Serial.printf("CMD: MOVETO %ld -> moving to %ld\n", value, stepper.targetPosition());
    // save new target
    savedPosition = stepper.targetPosition();
    EEPROM.put(0, savedPosition);
    EEPROM.commit();
    Serial.printf("Saved position to EEPROM: %ld\n", savedPosition);
  }
  else if (key == "SPEED")
  {
    // Set max speed (steps/sec)
    stepper.setMaxSpeed(value);
    Serial.printf("CMD: SPEED %ld -> speed=%g\n", value, stepper.maxSpeed());
    // save speed
    savedSpeed = value;
    EEPROM.put(sizeof(savedPosition), savedSpeed);
    EEPROM.commit();
    Serial.printf("Saved speed to EEPROM: %ld\n", savedSpeed);
  }
  else if (key == "ACCEL")
  {
    // Set acceleration (steps/sec²)
    stepper.setAcceleration(value);
    Serial.printf("CMD: ACCEL %ld -> accel=%g\n", value, stepper.acceleration());
    // save accel
    savedAccel = value;
    EEPROM.put(sizeof(savedPosition) + sizeof(savedSpeed), savedAccel);
    EEPROM.commit();
    Serial.printf("Saved accel to EEPROM: %ld\n", savedAccel);
  }
  else if (key == "SET0")
  {
    // Zero current position
    stepper.setCurrentPosition(0);
    Serial.println("CMD: SET0 -> position reset to 0");
    // save zero
    savedPosition = 0;
    EEPROM.put(0, savedPosition);
    EEPROM.commit();
    Serial.println("Saved position to EEPROM: 0");
  }
  else if (key == "GRIP")
  {
    // Set gripper servo angle
    servoGrip.write((int)value);
    Serial.printf("CMD: GRIP %ld -> angle=%ld\n", value, value);
  }
  else if (key == "ROTATE")
  {
    // Set gripper rotation servo angle
    servoRotate.write((int)value);
    Serial.printf("CMD: ROTATE %ld -> angle=%ld\n", value, value);
  }
  else
  {
    Serial.printf("Unknown command: '%s'\n", cmd.c_str());
  }
}

// ----------------------------------------------------------------------------
// ---- Setup -------------------------------------------------------------------
void setup()
{
  // Initialize Serial (USB CDC)
  Serial.begin(115200);
  delay(500);
  // Initialize EEPROM and load saved settings
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(0, savedPosition);
  EEPROM.get(sizeof(savedPosition), savedSpeed);
  EEPROM.get(sizeof(savedPosition) + sizeof(savedSpeed), savedAccel);
  Serial.printf("Loaded from EEPROM -> Position: %ld, Speed: %ld, Accel: %ld\n", savedPosition, savedSpeed, savedAccel);
  if (savedAccel <= 0)
  {
    long savedPosition = 0;
    long savedSpeed = 8000;
    long savedAccel = 2500;
    //save defaults if not set
    EEPROM.put(0, savedPosition);
    EEPROM.put(sizeof(savedPosition), savedSpeed);
    EEPROM.put(sizeof(savedPosition) + sizeof(savedSpeed), savedAccel);
    EEPROM.commit();
    Serial.println("EEPROM was empty or invalid, set defaults: Position=0, Speed=8000, Accel=2500");
  }

  // Apply saved settings

  // Configure stepper pins
  pinMode(PIN_STEP_EN, OUTPUT);
  digitalWrite(PIN_STEP_EN, LOW); // Enable driver (active LOW)
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);

  // Initialize AccelStepper defaults
  stepper.setMaxSpeed(savedSpeed);           // default max speed
  stepper.setCurrentPosition(savedPosition); // set initial position from EEPROM
  stepper.setAcceleration(savedAccel);       // default accel
// Attach gripper servos
#ifdef GRIP
  servoGrip.attach(PIN_GRIP);
  servoRotate.attach(PIN_GRIP_ROTATE);
  Serial.println("Gripper servos attached");
#endif

  // Initialize WiFi and ESP-NOW as slave (station)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // no need to connect to AP
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW init failed");
    while (true)
    {
      delay(100);
    }
  }
  // Register receive callback
  esp_now_register_recv_cb(onEspNowReceive);

  Serial.println("Stepper Slave Ready");
  Serial.println("Awaiting commands via ESP-NOW or Serial...");
  String localMac = WiFi.macAddress();
  Serial.print("Local MAC: ");
  Serial.println(localMac);
}

// ----------------------------------------------------------------------------
// ---- Main Loop ---------------------------------------------------------------
void loop()
{
  // 1) Handle incoming Serial commands (USB CDC)
  if (Serial.available())
  {
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() > 0)
    {
      executeCommand(line);
    }
  }

  // 2) Run stepper (non-blocking)
  stepper.run();
}
