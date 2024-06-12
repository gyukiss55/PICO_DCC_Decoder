//PWM_Control.cpp

#include <Arduino.h>

#include "PWM_Control.h"

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
bool direction = false;

void SetupPWM()
{
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
