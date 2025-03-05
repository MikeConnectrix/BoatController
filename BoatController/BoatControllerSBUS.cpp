#include "BoatControllerDefines.h"
#include "BoatControllerSBUS.h"
#include "BoatControllerServos.h"
#include "sbus.h"
#include <ArduinoJson.h>

extern BoatControllerServosClass bContServos;
extern JsonArray Servos;
extern JsonArray Channels;
extern JsonArray ServoTypes;
extern void WriteDebug(String msg);
extern bool debugRCChannels;
extern bool debugCompass;
extern bool debugTracking;


/* SbusRx object on Serial1 */
bfs::SbusRx sbus_rx(&Serial1);
uint16_t sbus_data[bfs::SbusRx::NUM_CH()];
uint16_t lastSBusData[bfs::SbusRx::NUM_CH()];
uint16_t sbus_data_min[bfs::SbusRx::NUM_CH()];
uint16_t sbus_data_max[bfs::SbusRx::NUM_CH()];


void BoatControllerSBUSClass::doWork() {

	if (sbus_rx.Read()) {
		for (JsonVariant value : Channels) {
			int channelID = value["channel"].as<int>();
			uint16_t newValue = sbus_rx.ch(channelID);
			sbus_data[channelID] = newValue;
			if (std::abs(lastSBusData[channelID] - sbus_data[channelID]) > 10) {
				if (value["cMax"].as<int>() < newValue) {
					value["cMax"].set(newValue + 1);
					value["cMin"].set(newValue - 1601);
				}
				if (value["cMin"].as<int>() > newValue) {
					value["cMin"].set(newValue - 1);
					value["cMax"].set(newValue + 1601);
				}
				
				float newPos = map(newValue, value["cMin"].as<int>(), value["cMax"].as<int>(), value["min"].as<int>(), value["max"].as<int>());

				bContServos.moveServo(channelID, value["servo"].as<int>(), newPos);
			}
			lastSBusData[channelID] = newValue;
		}
	}
}

void BoatControllerSBUSClass::init() {
	Serial.println("Initialising SBus interface");	
	pinMode(SBUSRX, INPUT);
    sbus_rx.Begin(SBUSRX, SBUSTX);	
}

