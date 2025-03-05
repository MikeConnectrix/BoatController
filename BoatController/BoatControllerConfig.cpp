#include "BoatControllerConfig.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include "FFat.h"
#include "SD.h"

extern DynamicJsonDocument config;
extern JsonArray Servos;
extern JsonArray Channels;
extern JsonArray ServoTypes;
extern void WriteDebug(String msg);
void BoatControllerConfigClass::init() {
	
}

void BoatControllerConfigClass::readConfig() {
	WriteDebug("Reading config file:");

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
				Channels = config["Channels"].as<JsonArray>();
				ServoTypes = config["SType"].as<JsonArray>();

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
	Serial.println("Config Saved!");
}

void BoatControllerConfigClass::saveConfigToFile(String fileName) {

	File configFile = FFat.open(fileName, FILE_WRITE);
	if (!configFile) {
		WriteDebug("- failed to open config file for writing");
	}
	else {
		serializeJson(config, configFile);
	}
	configFile.close();
}

void BoatControllerConfigClass::saveConfigToSDFile(String fileName) {
	WriteDebug("Saving config file to SD Card...");
	File configFile = SD.open(fileName, FILE_WRITE);
	if (!configFile) {
		WriteDebug("- failed to open config file for writing");
	}
	else {
		serializeJson(config, configFile);		
	}
	configFile.close();
	WriteDebug("Save Completed!!");
	
}

void BoatControllerConfigClass::restoreConfigFromSDFile(String fileName) {
	WriteDebug("Restoring config file from SD Card...");
	File sourceFile = SD.open(fileName);
	File destFile = FFat.open("/config.json", FILE_WRITE);
	static uint8_t buf[512];
	while (sourceFile.read(buf, 512)) {
		destFile.write(buf, 512);
	}
	destFile.close();
	sourceFile.close();
	readConfig();
	WriteDebug("Restore completed!");
}

void BoatControllerConfigClass::backupConfig() {
	saveConfigToFile("/config.bak");
}

JsonVariant BoatControllerConfigClass::getElement(String ElementName) {
	JsonVariant result = config[ElementName];
	return JsonVariant();
}
