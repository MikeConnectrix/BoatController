#include "BoatControllerConfig.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "FFat.h"

extern DynamicJsonDocument config;
extern JsonArray Servos;

void BoatControllerConfigClass::init() {
	
}

void BoatControllerConfigClass::readConfig() {
	Serial.println("Reading config file:");

	File file = FFat.open("/config.json");
	if (file) {
		DeserializationError err = deserializeJson(config, file);
		if (err) {
			Serial.print(F("deserializeJson() failed: "));
			Serial.println(err.c_str());
			file.close();
			deserializeJson(config, "{}");
			file = FFat.open("/config.bak");
			deserializeJson(config, file);
		}
		else {
			if (config["Params"]) {
				const char* boatname = config["Params"]["boat"];
				Serial.printf("Boat Name: %s\n", boatname);

				JsonArray controllers = config["Cont"].as<JsonArray>();

				Servos = config["Servo"].as<JsonArray>();

				Serial.printf("Attached Controllers:%d\n", controllers.size());
				for (JsonVariant value : controllers) {
					Serial.printf("Controller %d - %s : Number of Servos:%d\n", value["I2C"].as<int>(), value["dscn"].as<const char*>(), value["Servo"].size());
				}
				backupConfig();
			}
			else
				Serial.println("Error reading Params Block!");
		}
	}
	file.close();
}

void BoatControllerConfigClass::saveConfig() {
	saveConfigToFile("/config.json");
}

void BoatControllerConfigClass::saveConfigToFile(String fileName) {

	File configFile = FFat.open(fileName, FILE_WRITE);
	if (!configFile) {
		Serial.println("- failed to open config file for writing");
	}
	else {
		serializeJson(config, configFile);
	}
	configFile.close();
}

void BoatControllerConfigClass::backupConfig() {
	saveConfigToFile("/config.bak");
}

JsonVariant BoatControllerConfigClass::getElement(String ElementName) {
	JsonVariant result = config[ElementName];
	return JsonVariant();
}
