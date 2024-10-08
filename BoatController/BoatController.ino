#include "BoatControllerConfig.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <HttpsOTAUpdate.h>
#include <dummy.h>
#include <Arduino.h>
#include <SD.h>
#include "FFat.h"
#include "BoatControllerDefines.h"
#include "BoatControllerTFT.h"
#include "BoatControllerWifi.h"
#include "BoatControllerServos.h"
#include "BoatControllerSBUS.h"
#include "BoatControllerGPS.h"
#include "BoatControllerCompass.h"
#include "BoatControllerConfig.h"


BoatControllerWififClass bContWifi;
BoatControllerServosClass bContServos;
BoatControllerTFTClass bContTFT;
BoatControllerSBUSClass bContSBus;
BoatControllerGPSClass bContGPS;
BoatControllerCompassClass bContCompass;
BoatControllerConfigClass bContConfig;


float Pi = 3.14159;
float CompassHeading;

unsigned long bcCurrentMillis;
unsigned long bcLastCompassMillis;
unsigned long NextLED = LOW;
bool TFTConnected = true;
DynamicJsonDocument config(2048);
JsonArray Servos;

void ServoDoWork(void* pvParameters) {
    bContServos.init();
    bContSBus.init();
	
    for (;;) {
        bContSBus.doWork();
        bContServos.doWork();
		
        bcCurrentMillis = millis();

		if (bcCurrentMillis - bcLastCompassMillis > 1000) { // checks every 1 seconds
			digitalWrite(LED_2, NextLED);
			bcLastCompassMillis = bcCurrentMillis;
			bContCompass.doWork();
			if (NextLED == HIGH)
				NextLED = LOW;
			else
				NextLED = HIGH;
		}

		vTaskDelay(2);
    }
}


void WifiDoWork(void* pvParameters) {
    bContWifi.init();

    for (;;) {
        bContWifi.doWork();
        vTaskDelay(100);
    }
}

void TFTDoWork(void* pvParameters) {
    bContTFT.init();

    for (;;) {
        bContTFT.doWork();
        vTaskDelay(100);
    }
}

void GPSTask(void* pvParameters) {
    bContGPS.init();

    for (;;) {
        bContGPS.doWork();
        vTaskDelay(5000);

    }
}

void ReadConfig() {
	
   
}

void setup() {
    Serial.begin(115200);   
    Serial.println("Firmware Version 2.4...");

    if (!FFat.begin(FORMAT_SPIFFS_IF_FAILED)) {
		Serial.println("FFat Mount Failed");
		return;
	}

    bContConfig.init();

	bContConfig.readConfig();  

    bContCompass.init();
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);    

    if (!SD.begin()) {
        Serial.println("Card Mount Failed");
        TFTConnected = false;
    }

    xTaskCreate(
		ServoDoWork,   // Function that should be called
		"ServoDoWork", // Name of the task (for debugging)
		6144,		   // Stack size (bytes)
		NULL,		   // Parameter to pass
		10,			   // Task priority
		NULL		   // Task handle
	);

    if (TFTConnected) {
        xTaskCreate(
            TFTDoWork,    // Function that should be called
            "TFTDoWork",   // Name of the task (for debugging)
            4096,            // Stack size (bytes)
            NULL,            // Parameter to pass
            20,               // Task priority
            NULL             // Task handle
        );
    }

    
    xTaskCreate(
        WifiDoWork,    // Function that should be called
        "WifiDoWork",   // Name of the task (for debugging)
        4096,            // Stack size (bytes)
        NULL,            // Parameter to pass
        30,               // Task priority
        NULL             // Task handle
    );

    xTaskCreate(
        GPSTask,    // Function that should be called
        "GPSTask",   // Name of the task (for debugging)
        2048,            // Stack size (bytes)
        NULL,            // Parameter to pass
        40,               // Task priority
        NULL             // Task handle
    );

    
}


void loop() {
    
    vTaskDelay(100);
}
