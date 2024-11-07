#include "BoatControllerDefines.h"
#include <ArduinoJson.h>
#include <IBusBM.h>// 
// 
// 
extern JsonArray Servos;

#include "BoatControllerIBus.h"
const int IBusChannels = 8;
IBusBM IBus;
uint16_t ibus_data[IBusChannels];
uint16_t ibus_data_min[IBusChannels];
uint16_t ibus_data_max[IBusChannels];
uint16_t lastIBusData[IBusChannels];

#define servoPin SBUSRX;


void BoatControllerIBUSClass::init() {
	Serial.println("Initialising IBus interface:");	
	pinMode(SBUSRX, INPUT);
	IBus.begin(Serial1, IBUSBM_NOTIMER, SBUSRX, SBUSTX);
}

void BoatControllerIBUSClass::doWork() {	
	for (int8_t i = 0; i < IBusChannels; i++) {
		IBus.loop();
		uint16_t newValue = IBus.readChannel(i);
		ibus_data[i] = newValue;
		if (std::abs(lastIBusData[i] - ibus_data[i]) > 5) {					
			if (ibus_data_max[i] < newValue) {
				ibus_data_max[i] = newValue;
				ibus_data_min[i] = newValue-1000;			
			}
			if (ibus_data_min[i] > newValue) {
				ibus_data_min[i] = newValue;
			}	
			
			for (JsonVariant value : Servos) {
				if (value["RC"].as<int>() == i) {					
					float newPos = map(newValue, ibus_data_min[i], ibus_data_max[i], value["min"].as<int>(), value["max"].as<int>());
					Serial.printf("Channel %d : %.0f\n", i, newPos);			
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
