#include "BoatControllerDefines.h"
#include <ArduinoJson.h>
#include <IBusBM.h>// 
// 
// 
extern JsonArray Servos;

#include "BoatControllerIBus.h"
const int IBusChannels = 8;
IBusBM IBus;
int16_t ibus_data[IBusChannels];
int16_t lastIBusData[IBusChannels];

#define servoPin SBUSRX;


void BoatControllerIBUSClass::init() {
	/* IbusRx object on Serial1 */
	Serial.println("Initialising IBus interface:");
	pinMode(SBUSRX, INPUT);
	IBus.begin(Serial1, 2, SBUSRX, 9);
}

void BoatControllerIBUSClass::doWork() {	
	for (int8_t i = 0; i < IBusChannels; i++) {
		uint16_t newValue = IBus.readChannel(i);
		ibus_data[i] = newValue;
		if (std::abs(lastIBusData[i] - ibus_data[i]) > 5) {
				float sRatio = (newValue) / 1640.0;
				for (JsonVariant value : Servos) {
					if (value["RCChannel"].as<int>() == i) {
						float newPos = value["max"].as<int>() * sRatio;
						newPos = newPos * sRatio;
						if (newPos < value["min"].as<int>())
							newPos = value["min"].as<int>();
						if (newPos > value["max"].as<int>())
							newPos = value["max"].as<int>();
						value["target"].set(newPos);
						break;
					}
				}
		}
		lastIBusData[i] = newValue;
	}
}
