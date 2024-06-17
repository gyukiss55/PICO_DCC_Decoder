// PICO_Wifi.cpp


#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "PICO_Wifi.h"
#include "PICO_UART1_Def.h"

//char ssid[] = "RTAX999";       // your network SSID (name)
char ssid[] = "ASUS_98_2G";       // your network SSID (name)
char pass[] = "LiDoDa#959285$";   // your network password
//char ssid[] = "HUAWEI P30";       // your network SSID (name)
//char pass[] = "6381bf07b666";   // your network password

int status = WL_IDLE_STATUS;
WiFiUDP Udp;
unsigned int localPort = 2390;  // local port to listen on
WiFiServer server(80);

void SetupWifi()
{
    //Initialize serial and wait for port to open:

    // check for the WiFi module:
    if (WiFi.status() == WL_NO_MODULE) {
        Serial.println("Communication with WiFi module failed!");
        while (true);
    }

    // attempt to connect to WiFi network:
    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to SSID: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass);

        // wait 10 seconds for connection:
        delay(10000);
    }
    Serial.println("Connected to wifi");
    Serial.print("Local IP address: ");
    Serial.println(WiFi.localIP());
}

void SetupUDP()
{
    SetupWifi();
    Udp.begin(localPort);
    Serial.print("Listening on port ");
    Serial.println(localPort);
}

void SetupServer()
{
    SetupWifi();
    server.begin();
}


void LoopUDP() {
    int packetSize = Udp.parsePacket();
    if (packetSize) {
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remote = Udp.remoteIP();
        Serial.print(remote);
        Serial.print(", port ");
        Serial.println(Udp.remotePort());

        // read the packet into packetBufffer
        char packetBuffer[255];
        int len = Udp.read(packetBuffer, 255);
        if (len > 0) {
            packetBuffer[len] = 0;
        }
        Serial.println("Contents:");
        Serial.println(packetBuffer);

        // send a reply to the IP address and port that sent us the packet we received
        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write("Received your message");
        Udp.endPacket();
    }
}

void LoopServer(String& receivedStr)
{
    receivedStr = "";
    WiFiClient client = server.accept();
    if (!client) {
        return;
    }
    String recStr = "";
    if (client) {
        Serial.println("New client connected");
        String currentLine = "";
        while (client.connected()) {
            if (client.available()) {
            //if (client.accept()) {
                char c = client.read();
                recStr += c;
                Serial.write(c);
                if (c == '\n') {
                    if (currentLine.length() == 0) {
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-type:text/html");
                        client.println();

                        client.print("Hello from PICO W server! Rec:");
                        client.print(recStr);
                        client.println();
                        break;
                    }
                    else {
                        currentLine = "";
                    }
                }
                else if (c != '\r') {
                    currentLine += c;
                }
            }
        }
        client.stop();
        Serial.print("Received:");
        Serial.print(recStr);
        Serial.println("Client disconnected");
        receivedStr = recStr;
    }
}
