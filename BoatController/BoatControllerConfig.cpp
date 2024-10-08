#include "BoatControllerConfig.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "FFat.h"

extern DynamicJsonDocument config;

void BoatControllerConfigClass::init() {
	
}

void BoatControllerConfigClass::readConfig() {
	Serial.println("Reading config file:");

	File file = FFat.open("/config.json");
	if (file) {
		deserializeJson(config, file);
		const char* boatname = config["boatName"];
		Serial.printf("Boat Name: %s\n", boatname);
		JsonArray controllers = config["Controllers"].as<JsonArray>();
		Serial.printf("Attached Controllers:%d\n", controllers.size());
		for (JsonVariant value : controllers) {
			Serial.printf("Controller %d - %s : Number of Servos:%d\n", value["I2CAddress"].as<int>(), value["description"].as<const char*>(), value["Servos"].size());
		}
	}
	file.close();
}

void BoatControllerConfigClass::saveConfig() {
	File configFile = FFat.open("/config.json", FILE_WRITE);
	if (!configFile) {
		Serial.println("- failed to open config file for writing");
	}
	else {
		serializeJson(config, configFile);
	}
	configFile.close();
}

JsonVariant BoatControllerConfigClass::getElement(String ElementName) {
	JsonVariant result = config[ElementName];
	return JsonVariant();
}
