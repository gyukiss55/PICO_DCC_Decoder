//PWM_Control.cpp

#include <Arduino.h>

#include "PWM_Control.h"
#include "DCCWebCommandParser.h"

#define TIMEINCSPEED        50
#define TIMEDECSPEED        50
#define TIMESTOP          1000
#define TIMEFULLSPEED     2000

const int motorPWMPin1 = 16; // PWM1-for motor control
const int motorPWMPin2 = 17; // PWM2-for motor control
const int motorEnablePin = 18; // PWM-enable for motor control

int state = 0;
uint32_t prevMillis = 0;
int speed = 0;
int address = 0;
bool direction = false;

void SetupPWM(int addr)
{
    address = addr;
    pinMode(motorPWMPin1, OUTPUT);
    pinMode(motorPWMPin2, OUTPUT);
    pinMode(motorEnablePin, OUTPUT);

    digitalWrite(motorPWMPin1, LOW);
    digitalWrite(motorPWMPin2, LOW);
    digitalWrite(motorEnablePin, LOW);
}

void LoopPWM()
{
    uint32_t currentMillis = millis();
    switch (state) {
    case 0:
        if (currentMillis - prevMillis > TIMESTOP) {
            speed = 0;
            analogWrite(motorPWMPin1, speed);
            analogWrite(motorPWMPin2, speed);
            state++;
            prevMillis = currentMillis;
            digitalWrite(motorEnablePin, HIGH);
        }

        break;
    case 1:
    // Gradually increase the motor speed
        if (currentMillis - prevMillis > TIMEINCSPEED) {
            speed++;
            if (speed == 255)
                state++;
            if (direction)
                analogWrite(motorPWMPin1, speed);
            else
                analogWrite(motorPWMPin2, speed);
            prevMillis = currentMillis;
        }
        break;
    case 2:
        if (currentMillis - prevMillis > TIMEFULLSPEED) {
            state++;
            prevMillis = currentMillis;
        }
        break;
    case 3:
    // Gradually decrease the motor speed
        if (currentMillis - prevMillis > TIMEDECSPEED) {
            speed--;
            if (speed == 0) {
                state++;
                direction = !direction;
            }
            else {
                if (direction)
                    analogWrite(motorPWMPin1, speed);
                else
                    analogWrite(motorPWMPin2, speed);
            }
            prevMillis = currentMillis;
        }
        break;
    case 4:
        state = 0;
        digitalWrite(motorEnablePin, LOW);
        break;
    }
}


void PWMCommand (const String& receivedStr)
{
    if (receivedStr.length() > 0) {
        std::string commandStr (receivedStr.c_str());
        WebCommandParser parser (commandStr);
        Serial.print("PWMCommand begin:");
        Serial.println(commandStr.c_str ());
        if (parser.IsAlertStop()) {
            digitalWrite(motorEnablePin, LOW);
            speed = 0;
            direction = false;
            analogWrite(motorPWMPin1, speed);
            analogWrite(motorPWMPin2, speed);
        }
        else {
            bool forward = false;
            uint8_t speedBack;
            if (parser.GetDirectionAndSpeed(forward, speedBack)); {
                direction = !forward;
                speed = (speedBack & 0x1f) * 8;
                if (direction)
                    analogWrite(motorPWMPin1, speed);
                else
                    analogWrite(motorPWMPin2, speed);
                digitalWrite(motorEnablePin, HIGH);

                Serial.print("Direction:");
                if (direction)
                    Serial.println("backward");
                else 
                    Serial.println("forward");
                Serial.print("Speed:");
                Serial.println(speed, DEC);
            }

        }
        Serial.println("PWMCommand end.");
    }
}