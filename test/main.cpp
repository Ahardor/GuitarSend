#include <esp_now.h>
#include <esp_wifi.h>
#include <driver/adc.h>
#include <driver/dac.h>
// #include <esp32-hal-timer.h>

#include <WiFi.h>

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0xD4, 0x8A, 0xFC, 0x99, 0x00, 0x28};

esp_now_peer_info_t peerInfo;


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(status){
    Serial.println("PACKET ERROR!");
  }
}

typedef struct struct_message {
  uint8_t sound[250];
} struct_message;

struct_message SoundOutcome;
void setup() {
  // dac_output_enable(DAC_CHANNEL_1);
  pinMode(34, INPUT);
  Serial.begin(115200);

  esp_netif_init();
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_mode(WIFI_MODE_STA);
  esp_wifi_set_bandwidth(WIFI_IF_STA, WIFI_BW_HT40);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_ps(WIFI_PS_NONE);
  esp_wifi_start();

  if (esp_now_init() != ESP_OK) {
    //Serial.println("Error initializing ESP-NOW");
    return;
  }

  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    //Serial.println("Failed to add peer");
    return;
  }

  esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_54M);
  esp_now_register_send_cb(OnDataSent);

}

int pos = 0;
 
void loop() {
  if(pos < 250) {
    int val = analogRead(34) >> 4;
    if(abs(val - 120) < 3){
      SoundOutcome.sound[pos] = 120;
    }

    else {
      SoundOutcome.sound[pos] = val;
    }
    // SoundOutcome.sound[pos] = val;

    Serial.println(SoundOutcome.sound[pos]);
    // dac_output_voltage(DAC_CHANNEL_1, val);
    pos++;
  }

  else {
    esp_now_send(broadcastAddress, (uint8_t *) &SoundOutcome, sizeof(SoundOutcome));
    pos = 0;
    // delay(100);
  }
  
  delayMicroseconds(125);
}
