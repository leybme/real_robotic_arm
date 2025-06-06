#include <WiFi.h>
#include <esp_now.h>

// ─── CONFIGURE SLAVE MAC HERE (change to your slave’s MAC) ───
// e.g. { 0x24, 0x0A, 0xC4, 0x12, 0x34, 0x56 }
// R1
uint8_t R1_MAC[6] = {0xCC, 0xBA, 0x97, 0x60, 0x23, 0x24};
String R1_NAME = "R1";
// Z
uint8_t Z_MAC[6] = {0x94, 0xA9, 0x90, 0x43, 0xD0, 0xA0};
String Z_NAME = "Z";

// R2
uint8_t R2_MAC[6] = {0xCC, 0xBA, 0x97, 0x61, 0xD0, 0x0C};
String R2_NAME = "R2";
// uint8_t SLAVE_MAC[6] = { 0xCC, 0xBA, 0x97, 0x61, 0xD0, 0x0C };

// ─── CALLBACK FOR SEND STATUS (optional) ─────────────────────
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           mac_addr[0], mac_addr[1], mac_addr[2],
           mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet to: ");
  Serial.print(macStr);
  //compare with R1_MAC, Z_MAC, R2_MAC
  if (memcmp(mac_addr, R1_MAC, 6) == 0)
    Serial.print(" (" + R1_NAME + ")");
  else if (memcmp(mac_addr, Z_MAC, 6) == 0)
    Serial.print(" (" + Z_NAME + ")");
  else if (memcmp(mac_addr, R2_MAC, 6) == 0)
    Serial.print(" (" + R2_NAME + ")");
  else
    Serial.print(" (Unknown)");
  Serial.print(" → Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}
/**
 * Send a null-terminated C string via ESP-NOW to the specified peer.
 */
void sendMessage(const uint8_t *peerMac, const char *msg)
{
  // Calculate length without including null terminator
  size_t len = strlen(msg);
  esp_err_t result = esp_now_send(peerMac, (const uint8_t *)msg, len);
  if (result == ESP_OK)
  {
    Serial.print("Sent: ");
    Serial.println(msg);
  }
  else
  {
    Serial.printf("Error sending: code %d\n", result);
  }
}

// ─── SETUP ────────────────────────────────────────────────────
void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println();

  // Put WiFi in STA mode (required for ESP-NOW)
  WiFi.mode(WIFI_STA);
  Serial.print("Master MAC: ");
  Serial.println(WiFi.macAddress());
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error: ESP-NOW Init Failed");
    while (true)
    {
      delay(100);
    }
  }

  // Register the callback to get send status (optional)
  esp_now_register_send_cb(onDataSent);

  // Register up to 3 peers with encryption disabled
  {
    const uint8_t* peers[3] = { R1_MAC, Z_MAC, R2_MAC };
    esp_now_peer_info_t peerInfo = {};
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    for (int i = 0; i < 3; ++i) {
      memcpy(peerInfo.peer_addr, peers[i], 6);
      if (!esp_now_is_peer_exist(peers[i])) {
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
          Serial.printf("Failed to add peer %d\n", i);
          while (true) { delay(100); }
        }
      }
    }
  }

  Serial.println("Ready. Type a command and press Enter →");
}

// ─── LOOP ─────────────────────────────────────────────────────
void loop()
{
  // Check if the user typed something on USB-Serial
  if (Serial.available())
  {
    // Read one line (up to newline)
    String line = Serial.readStringUntil('\n');
    line.trim();
    if (line.length() == 0)
      return;

    // Convert String to raw bytes
    size_t len = line.length();
    const uint8_t *dataPtr = (const uint8_t *)line.c_str();

    // Send via ESP-NOW
    sendMessage(R2_MAC, line.c_str());
  }

  // (no blocking here—just keep looping)
}
