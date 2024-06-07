// #define CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED 0
// #define CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED 0

#include <Wifi.h>
#include <driver/adc.h>
#include <driver/dac.h>
#include <esp32-hal-timer.h>
#include <PubSubClient.h>
#include <CircularBuffer.hpp>
#include <WebSocketsClient.h>


const int buffer_size = 1000;

uint8_t input_buffer[buffer_size];
uint8_t output_buffer[buffer_size];

volatile int pos=0;
volatile uint8_t sent=1;
volatile uint8_t dataReady=0;
volatile uint8_t pin_state=0;

hw_timer_t *My_timer = NULL;

const char* ssid = "RockusWIFI";
const char* password = "Alvard86";

const char* address = "192.168.1.109";
const int port = 1883;
const char* topic = "sound_in";

WiFiClient espClient;
PubSubClient client(espClient);

void IRAM_ATTR sampleADC(){
    input_buffer[pos++] = adc1_get_raw(ADC1_CHANNEL_6) >> 4;
    if(pos >= buffer_size){
        pos = 0;
        if(sent){
            memcpy(output_buffer, input_buffer, buffer_size);
            dataReady=1;
            sent=0;
        }
    }
}

void setup() {
    Serial.begin(115200);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500);
    }

    client.setBufferSize(buffer_size + 20);
    client.setServer(address, port);

    My_timer = timerBegin(0, 8, true);
    timerAttachInterrupt(My_timer, &sampleADC, true);
    timerAlarmWrite(My_timer, 625, true);
    timerAlarmEnable(My_timer);
}

void loop() {
    if (!client.connected()) {
            while (!client.connected()) {
                Serial.print("MQTT connecting ...");
                String clientId = "Sender";
                if (client.connect(clientId.c_str())) {
                    Serial.println("connected");
                    Serial.println("Entering loop...");
                    timerAlarmEnable(My_timer);
                } else {
                    Serial.print("failed, status code =");
                    Serial.print(client.state());
                    Serial.println("try again in 5 seconds");
                    /* Wait 5 seconds before retrying */
                    delay(5000);
                }
            }
        }
        else if(dataReady){
            client.publish(topic, output_buffer, buffer_size);
            sent=1;
            dataReady=0;
        }
}