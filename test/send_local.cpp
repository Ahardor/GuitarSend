#define CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED 0
#define CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED 0

#include <esp_now.h>
#include <esp_wifi.h>
#include <driver/adc.h>
#include <driver/dac.h>
#include <esp32-hal-timer.h>
#include <HardwareSerial.h>

uint8_t broadcastAddress[] = {0xD4, 0x8A, 0xFC, 0x99, 0x00, 0x28};

uint8_t input_buffer[250]={};
uint8_t output_buffer[250];

volatile uint8_t i=0;
volatile uint8_t sent=1;
volatile uint8_t dataReady=0;
volatile uint8_t pin_state=0;

hw_timer_t *My_timer = NULL;

esp_now_peer_info_t peerInfo;


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if(status){
    Serial.println("PACKET ERROR!");
  }
}

void IRAM_ATTR sampleADC(){
  input_buffer[i++] = adc1_get_raw(ADC1_CHANNEL_6) >> 4;
  // Serial.println(input_buffer[i-1]);
  if(i==250){
    i=0;
    if(sent){
      memcpy(output_buffer, input_buffer, 250);
      dataReady=1;
      sent=0;
    }
  }
}
 
void setup() {
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
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_54M);
  esp_now_register_send_cb(OnDataSent);
  
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  My_timer = timerBegin(0, 8, true);
  timerAttachInterrupt(My_timer, &sampleADC, true);
  timerAlarmWrite(My_timer, 500, true);

  Serial.println("Entering loop...");
  timerAlarmEnable(My_timer);
}
 
void loop() {
  if(dataReady){
    esp_err_t result = esp_now_send(broadcastAddress, output_buffer, sizeof(output_buffer));
    sent=1;
    dataReady=0;
    
  }
}