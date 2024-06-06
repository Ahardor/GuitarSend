#define CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED 0
#define CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED 0

#include <Wifi.h>
#include <driver/adc.h>
#include <driver/dac.h>
#include <esp32-hal-timer.h>
#include <WebSocketsClient.h>
#include <WebServer.h>
#include "ADPCM.h"

using namespace adpcm_ffmpeg;

const int buffer_size = 249;

int16_t input_buffer[buffer_size];
int16_t output_buffer[buffer_size];

volatile int i=0;
volatile uint8_t sent=1;
volatile uint8_t dataReady=0;
volatile uint8_t pin_state=0;

hw_timer_t *My_timer = NULL;

const char* ssid = "RockusWIFI";
const char* password = "Alvard86";

const char* address = "192.168.1.109";
const int port = 8080;
const char* path = "/ws";

ADPCMEncoder encoder{AV_CODEC_ID_ADPCM_IMA_WAV};
ADPCMDecoder decoder{AV_CODEC_ID_ADPCM_IMA_WAV};
int frame_size = 0;

void displayPacket(AVPacket& packet) {
    Serial.println("Packet: ");
    for (int j = 0; j < packet.size; j++) {
        if (j % 16 == 0) Serial.println();
        Serial.printf("%x ", packet.data[j]);
    }
    Serial.println();
}

void displayResult(AVFrame& frame) {
  // print the result
  int16_t* data = (int16_t*) frame.data[0];
  size_t frames = frame.nb_samples  ;
  for (int j = 0; j < frames; j++) {
    Serial.printf("%d ", data[j]);
  }
}

// WebSocketsClient webSocket;

void IRAM_ATTR sampleADC(){
    input_buffer[i++] = adc1_get_raw(ADC1_CHANNEL_6) >> 4;
    if(i == buffer_size){
        i = 0;
        if(sent){
            memcpy(output_buffer, input_buffer, buffer_size);
            dataReady++;
            // sent=0;
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

    // webSocket.setExtraHeaders("Origin: Sender");
    // webSocket.begin(address, port, path);

    // webSocket.setReconnectInterval(5000);

    if (!encoder.begin(20000, 1)) {
        Serial.println("encoder not supported");
        return;
    }

    if (!decoder.begin(20000, 1)) {
        Serial.println("decoder not supported");
        return;
  }

    frame_size = encoder.frameSize();

    My_timer = timerBegin(0, 8, true);
    timerAttachInterrupt(My_timer, &sampleADC, true);
    timerAlarmWrite(My_timer, 1000, true); 

    Serial.println("Entering loop...");
    timerAlarmEnable(My_timer);
}

void loop() {
    // webSocket.loop();
    if(dataReady){
        

        timerAlarmDisable(My_timer);
        // timerDetachInterrupt(My_timer);

        Serial.println("Input buffer: ");
        for(int i = 0; i < buffer_size; i++){
            Serial.printf("%d ", input_buffer[i]);
        }
        Serial.println();

        unsigned long t = micros();

        AVPacket& packet = encoder.encode(input_buffer, buffer_size);
        // displayPacket(packet);

        AVFrame& frame = decoder.decode(packet);
        // displayResult(frame);

        // Serial.println();

        Serial.printf("Compress time: %lu\n", micros() - t);

        dataReady = 0;
        encoder.end();
    }

    // if(dataReady){
    //     webSocket.sendBIN(output_buffer, sizeof(output_buffer));
    //     Serial.println("Data sent");
    //     // esp_err_t result = esp_now_send(broadcastAddress, output_buffer, sizeof(output_buffer));
    //     sent=1;
    //     dataReady=0;
    // }
}