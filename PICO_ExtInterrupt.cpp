//PICO_ExtInterrupt.cpp

#include <Arduino.h>
//#include <FreeRTOS.h>
//#include <Arduino_FreeRTOS.h>
//#include <semphr.h>

#include "PICO_ExtInterrupt.h"

#define MicroSecBufferLength 1024

//SemaphoreHandle_t mutex;

volatile bool pingPongState = false;
volatile unsigned long microSecBuffer1[MicroSecBufferLength];
volatile unsigned long microSecBuffer2[MicroSecBufferLength];
volatile unsigned long microSecBufferIndex1 = 0;
volatile unsigned long microSecBufferIndex2 = 0;

// Interrupt service routine (ISR)
void handleInterrupt() {
	unsigned long us = micros();
	/*if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)*/ {
		if (!pingPongState) {
			microSecBuffer1[microSecBufferIndex1++] = us;
			if (microSecBufferIndex1 >= MicroSecBufferLength) {
				pingPongState = !pingPongState;
				microSecBufferIndex2 = 0;
			}
		}
		else {
			microSecBuffer2[microSecBufferIndex2++] = us;
			if (microSecBufferIndex2 >= MicroSecBufferLength) {
				pingPongState = !pingPongState;
				microSecBufferIndex1 = 0;
			}

		}
		//xSemaphoreGive(mutex); // Release the mutex
	}
}

void ExtInterruptSetup(int interruptPin)
{
	pinMode(interruptPin, INPUT_PULLUP); // Set the interrupt pin as input with a pull-up resistor

	// Attach the interrupt to the interrupt pin, looking for a FALLING edge (button press)
	attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, CHANGE);

}

unsigned long GetLastMicros(unsigned long* buffer, unsigned long size) 
{
	/*if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE)*/ {
		bool pps = pingPongState;
		unsigned long ppi1 = microSecBufferIndex1;
		unsigned long ppi2 = microSecBufferIndex2;
		pingPongState = !pingPongState;
		if (!pingPongState) {
			microSecBufferIndex1 = 0;
			microSecBufferIndex2 = 0;
		}
		else {
			microSecBufferIndex2 = 0;
			microSecBufferIndex1 = 0;
		}
		//xSemaphoreGive(mutex); // Release the mutex

		unsigned long ret = 0;
		if (!pps) {
			unsigned long ppi10 = 0;
			if (ppi1 >= size)
				ppi10 = ppi1 - size;
			for (unsigned long i = 0; i < size; i++) {
				if (ppi1 > 0)
					ppi1--;
        else
          break;
				buffer[i] = microSecBuffer1[ppi10 + i];
				ret++;
			}
		}
		else {
			unsigned long ppi20 = 0;
			if (ppi2 >= size)
				ppi20 = ppi2 - size;
  		for (unsigned long i = 0; i < size; i++) {
				if (ppi2 > 0)
					ppi2--;
        else
          break;
				buffer[i] = microSecBuffer2[ppi20 + i];
				ret++;
			}

		}
		return ret;
	}

}

#define PrintSize 1024
unsigned long microSecPrintBuffer[PrintSize];

void PrintSample()
{
	int ret =  (int)GetLastMicros(microSecPrintBuffer, 128);
	String str = "PrintSample " + String (ret, DEC);
	for (int i = 0; i < ret - 1; i++) {
		if (i == 0)
			str += "#" + String(microSecPrintBuffer[i], DEC) + "#";
		str += "," + String(microSecPrintBuffer[i + 1] - microSecPrintBuffer[i], DEC);
	}
	Serial.println(str);
}

void PrintStatistic()
{
#define STATISTIC_RANGE 128
	unsigned long statisticMap[STATISTIC_RANGE];
	memset(statisticMap, 0, sizeof(statisticMap));
	int ret = (int)GetLastMicros(microSecPrintBuffer, PrintSize);
	String str = "PrintStatistic " + String(ret, DEC) + " ";

	for (int i = 0; i < ret - 1; i++) {
		unsigned long v = microSecPrintBuffer[i + 1] - microSecPrintBuffer[i];
		if (v < STATISTIC_RANGE)
			statisticMap[v]++;
		else
			statisticMap[STATISTIC_RANGE - 1]++;
	}
	for (int i = 0; i < STATISTIC_RANGE; i++) {
		if (statisticMap[i] > 0) {
			str += String(i, DEC) + "-" + String(statisticMap[i], DEC) + ", ";
		}
	}
	Serial.println(str);
}

