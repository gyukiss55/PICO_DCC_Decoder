
#include <list>
#include "PICO_ExtInterrupt.h"
#include "PWM_Control.h"
#include "PICO_Wifi.h"
#include "PICO_UART1.h"


#define version "PICO Decoder V.1.01"
#define DECODER_ADDRESS 12

void setup() {
	Serial.begin(115200);
	//while (!Serial) {
		; // wait for serial port to connect. Needed for native USB port only
	//}
	delay(2000);

	SetupUART1 ();

	Serial.println(version);
	pinMode(LED_BUILTIN, OUTPUT);

	//ExtInterruptSetup (13);

	SetupPWM (DECODER_ADDRESS);
	SetupServer ();
}

void loopBlink()
{
	static uint32_t tsPrev = 0;
	static bool state = false;
	uint32_t ts = millis();
	if (state) {
		if (ts - tsPrev >= 900) {
			state = !state;
			digitalWrite(LED_BUILTIN, HIGH);
			tsPrev = ts;
			Serial.print('.');
		}
	}
	else {
		if (ts - tsPrev >= 100) {
			state = !state;
			digitalWrite(LED_BUILTIN, LOW);
			tsPrev = ts;
		}
	}
}

void loopDecode()
{
	static std::list<String> strList;
	static uint32_t tsPrev = 0;
	uint32_t ts = millis();
	if (ts - tsPrev >= 20) {
		String str;
		DecodeCommand(str, false);
		if (str.length () > 0)
			strList.push_back(str);
		tsPrev = ts;
	}
	if (strList.size() >= 10) {
		std::list<String> strList2 = strList;
		strList.clear();
		for (String str : strList2)
		{
			Serial.println(str);
		}
	}

}

void loop() {
	LoopUART1 ();

	loopBlink ();
	//loopDecode (); 
	//LoopPWM ();
    String receivedStr;
	LoopServer (receivedStr);
    PWMCommand (receivedStr);
}             
