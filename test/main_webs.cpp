#define CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED 0
#define CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED 0

#include <Wifi.h>
#include <driver/adc.h>
#include <driver/dac.h>
#include <esp32-hal-timer.h>
#include <WebSocketsClient.h>
#include <WebServer.h>

const int buffer_size = 500;

byte input_buffer[buffer_size];
byte output_buffer[buffer_size];

volatile int pos=0;
volatile uint8_t sent=1;
volatile uint8_t dataReady=0;
volatile uint8_t pin_state=0;

hw_timer_t *My_timer = NULL;

const char* ssid = "RockusWIFI";
const char* password = "Alvard86";

const char* address = "192.168.1.109";
const int port = 8080;
const char* path = "/ws";

WebSocketsClient webSocket;

void IRAM_ATTR sampleADC(){
    input_buffer[pos++] = adc1_get_raw(ADC1_CHANNEL_6);
    if(pos == buffer_size){
        pos = 0;
        if(sent){
            memcpy(output_buffer, input_buffer, buffer_size);
            dataReady++;
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

    webSocket.setExtraHeaders("Origin: Sender");
    webSocket.begin(address, port, path);

    webSocket.setReconnectInterval(5000);

    My_timer = timerBegin(3, 8, true);
    timerAttachInterrupt(My_timer, &sampleADC, true);
    timerAlarmWrite(My_timer, 1000, true); 

    Serial.println("Entering loop...");
    timerAlarmEnable(My_timer);
}

void loop() {
    webSocket.loop();

    if(dataReady){
        // for(int i=0; i<buffer_size; i+=2){
        //     Serial.printf("%d%d ", output_buffer[i], output_buffer[i + 1]);
        // }
        // Serial.println();
        // Serial.println("--------------------------------------------------------------------");

        

        // for(int i=0; i<buffer_size; i++){
        //     Serial.printf("%d ", testBuf[i]);
        // }
        // Serial.println("\n");

        webSocket.sendBIN(output_buffer, buffer_size);
        sent=1;
        dataReady=0;
    }
}