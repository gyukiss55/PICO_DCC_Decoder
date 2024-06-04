
#include "PICO_ExtInterrupt.h"


#define version "PICO Decoder V.0.01"


void setup() {
	Serial.begin(115200);
	/*while (Serial () == nullptr) */{
		delay (2000);
	}
	Serial.println(version);
	pinMode(LED_BUILTIN, OUTPUT);

	ExtInterruptSetup (13);
}


void loop() {
	digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
	delay(100);                      // wait for a second
	digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
	delay(900);
 
	String str = String(millis(), DEC) + "sec";

	Serial.println(str);  

	PrintSample (); 
}             
