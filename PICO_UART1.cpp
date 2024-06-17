//PICO_UART1.cpp

#include <Arduino.h>
#include "PICO_UART1.h"

// Define UART1 pins
#define UART1_TX 8
#define UART1_RX 9

// Initialize UART1
//HardwareSerial Serial1(UART1);

void SetupUART1() {
    Serial2.setRX(UART1_RX);
    Serial2.setTX(UART1_TX);
    Serial2.begin(115200);
    // Initialize UART1
    //Serial1.begin(115200, SERIAL_8N1, UART1_RX, UART1_TX);
    Serial.println("UART2 initialized");
    Serial2.println("UART2 initialized");
}

void LoopUART1() {
    // Check if data is available on UART1
    if (Serial2.available()) {
        // Read the data and echo it to the serial monitor
        String receivedData = Serial2.readString();
        Serial.print("Received on UART2: ");
        Serial.println(receivedData);
        Serial2.print("Received on UART2: ");
        Serial2.println(receivedData);
    }

}