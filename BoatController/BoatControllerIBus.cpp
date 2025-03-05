#include "BoatControllerDefines.h"
#include "BoatControllerServos.h"
#include <ArduinoJson.h>
#include <IBusBM.h>// 
// 
// 
extern JsonArray Servos;
extern JsonArray Channels;
extern JsonArray ServoTypes;
extern bool debugRCChannels;
extern bool debugCompass;
extern bool debugTracking;
extern BoatControllerServosClass bContServos;

#include "BoatControllerIBus.h"
const int IBusChannels = 8;
IBusBM IBus;
uint16_t ibus_data[IBusChannels];
uint16_t lastIBusData[IBusChannels];
extern void WriteDebug(String msg);

#define servoPin SBUSRX;


void BoatControllerIBUSClass::init() {
	Serial.println("Initialising IBus interface:");	
	pinMode(SBUSRX, INPUT);
	IBus.begin(Serial1, IBUSBM_NOTIMER, SBUSRX, SBUSTX);
}

void BoatControllerIBUSClass::doWork() {	
	IBus.loop();

	for (JsonVariant value : Channels) {
		int channelID = value["channel"].as<int>();
		uint16_t newValue = IBus.readChannel(channelID);
		ibus_data[channelID] = newValue;
		if (newValue != 0) {
			// Has the Channel Data changed
			// Store the IBus min and max values for this channel
			if (value["cMax"].as<int>() < newValue) {
				value["cMax"].set(newValue + 1);
				value["cMin"].set(newValue - 1001);
			}
			if (value["cMin"].as<int>() > newValue) {
				value["cMin"].set(newValue - 1);
				value["cMax"].set(newValue + 1001);
			}
			
			float newPos = map(newValue, value["cMin"].as<int>(), value["cMax"].as<int>(), value["min"].as<int>(), value["max"].as<int>());

			bContServos.moveServo(channelID, value["servo"].as<int>(), newPos);		

			lastIBusData[channelID] = newValue;
		}
	}
}	

