//PICO_ExtInterrupt.cpp

#include <Arduino.h>
//#include <FreeRTOS.h>
//#include <Arduino_FreeRTOS.h>
//#include <semphr.h>

#include "PICO_ExtInterrupt.h"

#define MicroSecBufferLength 2048

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

#define PrintSize 512
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


#define BIT1MAX 255

#define BIT1HALF 58
#define BIT0HALF 100
#define BITHALFDELTA 10

#define BIT0 0
#define BIT1 1
#define BITERROR 2
#define BIT0LONG 3

#define PREAMBLELENGHT 14*2

#define DATABUFFERLENGTH 8


int BitDetec (uint8_t v)
{
	if ((v > (BIT0HALF - BITHALFDELTA)) && (v < (BIT0HALF + BITHALFDELTA))) {
		return BIT0; // bit 0 100 uSec
	}
	if ((v > (BIT1HALF - BITHALFDELTA)) && (v < (BIT1HALF + BITHALFDELTA))) {
		return BIT1; // bit 1 58 uSec
	}
	if (v == BIT1MAX) {
		return BIT0LONG; // bit max 9900 uSec
	}
	return BITERROR;	// error
}


uint32_t DecodeCommand(String& result, uint8_t* ptrBuffer, uint32_t sizeBuffer, bool debugPrint /*= false*/)
{
	static int phase = 0;
	static int phaseIndex = 0;

	static uint8_t dataBuffer[DATABUFFERLENGTH];
	static uint32_t dataBufferIndex = 0;
	static uint32_t dataBitIndex = 0;

	uint32_t i = 0;

	for (; i < sizeBuffer - 1; i++) {
		int b0 = BitDetec(ptrBuffer[i]);
		int b1 = BitDetec(ptrBuffer[i + 1]);
		//if (debugPrint) Serial.print(b0,DEC);
		if (b0 == BITERROR || b1 == BITERROR) {
			phase = 0;
			phaseIndex = 0;
			if (debugPrint) 
				Serial.print("!E1!");
			continue;
		}
		if (phase == 0) {// preamble 14 pc bit 1
			if (b0 == BIT1 && b1 == BIT1) {
				phaseIndex++;
				continue;
			}
			if (b0 == BIT1 && b1 == BIT0 && phaseIndex >= PREAMBLELENGHT) {
				phase = 1;
				phaseIndex = 0;
				continue;
			}
			phase = 0;
			phaseIndex = 0;
			if (debugPrint)
				Serial.print("@");
			continue;
		}
		if (phase == 1) {// preamble end bit 0
			if (b0 == BIT0 && b1 == BIT0) {
				phase = 2;
				dataBufferIndex = 0;
				dataBitIndex = 0;
				i++;
				continue;
			}
			phase = 0;
			phaseIndex = 0;
			Serial.print("!E3!");
			continue;
		}
		if (phase == 2) {// data
			if (b0 == b1) {
				if (dataBitIndex == 0)
					dataBuffer[dataBufferIndex] = 0;
				if (dataBitIndex < 8) {
					dataBuffer[dataBufferIndex] |= (b0 << (7 - dataBitIndex));
					dataBitIndex++;
				} else
				if (dataBitIndex == 8) {
					dataBufferIndex++;
					dataBitIndex = 0;
					if (b0 == BIT1) {// end of error detect
						phase = 0;
						phaseIndex = 0;
						String strDebug = "Detected:";
						result += "{";
						for (int k = 0; k < dataBufferIndex; k++) {
							if (debugPrint) strDebug += String(dataBuffer[k], HEX) + ",";
							result += String(dataBuffer[k], HEX) + ",";
						}
						if (debugPrint) Serial.println(strDebug);
						result += "}";
					}
				}
				i++;

				continue;
			}
			phase = 0;
			phaseIndex = 0;
			if (debugPrint) Serial.print("!E4!");
			continue;
		}
	}
	return i;
}

uint32_t DecodeCommand2(uint8_t* ptrBuffer, uint32_t sizeBuffer)
{
	String strDebug = "Decode:";
	for (uint32_t i = 0; i < sizeBuffer; i++) {
		strDebug += String(BitDetec(ptrBuffer[i]), DEC);
	}
	Serial.println(strDebug);
	return sizeBuffer;
}

void DecodeCommand(String& result, bool debugPrint /*= false*/)
{
#define HALFBITBUFFER_SIZE MicroSecBufferLength
	static unsigned long prevMicroSec = 0;
	static uint8_t halfBitBuffer[HALFBITBUFFER_SIZE];
	static uint32_t halfBitBufferIndex = 0;

	result = "";
	int ret = (int)GetLastMicros(microSecPrintBuffer, PrintSize);
	String strDebug = "DecodeCommand " + String(ret, DEC) + " ";

	if (ret > 1) {
		result += String(microSecPrintBuffer[0], DEC) + "/";
		if (debugPrint) {
			strDebug += String(microSecPrintBuffer[0], DEC) + "-";
			strDebug += String(microSecPrintBuffer[ret - 1], DEC) + "=";
			strDebug += String(microSecPrintBuffer[ret - 1] - microSecPrintBuffer[0], DEC) + "/";
			strDebug += String(prevMicroSec, DEC) + ".";
		}
	}

	for (int i = 0; i < ret; i++) {
		unsigned long v = 0;
		if (prevMicroSec > 0) {
			v = microSecPrintBuffer[i] - prevMicroSec;
			if (v > BIT1MAX)
				v = BIT1MAX;
			halfBitBuffer[halfBitBufferIndex] = (uint8_t)v;
			halfBitBufferIndex++;
			if (halfBitBufferIndex >= HALFBITBUFFER_SIZE) {
				if (debugPrint) 
					strDebug += "\r\nBuffer FULL!\r\n";
				for (int j = 0; j < HALFBITBUFFER_SIZE; j++) {
					//if (debugPrint) strDebug += String(halfBitBuffer[j], DEC) + ",";
				}
			}
		}
		prevMicroSec = microSecPrintBuffer[i];
	}

	if (ret > 1 && halfBitBufferIndex > 1) {
		uint32_t index = DecodeCommand(result, halfBitBuffer, halfBitBufferIndex, debugPrint);
		if (debugPrint) {
			strDebug += String(index, DEC) + ",";
			strDebug += String(halfBitBufferIndex, DEC) + ",";
		}
		if (index < halfBitBufferIndex) {
			for (uint32_t i = 0; i < halfBitBufferIndex - index; i++) {
				halfBitBuffer[i] = halfBitBuffer[index + i];
			}
			halfBitBufferIndex -= index;
		}
		else
			halfBitBufferIndex = 0;
	}
	if (debugPrint) {
		Serial.println(strDebug);
	}
}



