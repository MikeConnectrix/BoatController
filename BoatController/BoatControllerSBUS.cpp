#include "BoatControllerDefines.h"
#include "BoatControllerSBUS.h"
#include "sbus.h"
#include <ArduinoJson.h>

extern JsonArray Servos;

/* SbusRx object on Serial1 */
bfs::SbusRx sbus_rx(&Serial1);
uint16_t sbus_data[bfs::SbusRx::NUM_CH()];
uint16_t lastSBusData[bfs::SbusRx::NUM_CH()];
uint16_t sbus_data_min[bfs::SbusRx::NUM_CH()];
uint16_t sbus_data_max[bfs::SbusRx::NUM_CH()];


void BoatControllerSBUSClass::doWork() {
	if (sbus_rx.Read()) {
		for (int8_t i = 0; i < bfs::SbusRx::NUM_CH(); i++) {
			uint16_t newValue = sbus_rx.ch(i);
			sbus_data[i] = newValue;
			if (std::abs(lastSBusData[i] - sbus_data[i]) > 5) {
				if (sbus_data_max[i] < newValue) {
					sbus_data_max[i] = newValue;
					sbus_data_min[i] = newValue - 1000;
				}
				if (sbus_data_min[i] > newValue) {
					sbus_data_min[i] = newValue;
				}

				for (JsonVariant value : Servos) {
					if (value["RC"].as<int>() == i) {
						int target = map(newValue, sbus_data_min[i], sbus_data_max[i], value["min"].as<int>(), value["max"].as<int>());
						value["target"].set(target);
					}
				}
			}
			lastSBusData[i] = newValue;
		}
	}
}
void BoatControllerSBUSClass::init() {
	Serial.println("Initialising SBus interface");	
	pinMode(SBUSRX, INPUT);
    sbus_rx.Begin(SBUSRX, SBUSTX);	
}

