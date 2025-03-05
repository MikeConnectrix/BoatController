#include "BoatControllerTFTScreenInput.h"
#include "BoatControllerTFTScreen.h"
#include "BoatControllerPWM.h"
#include "BoatControllerIBus.h"
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
#include "BoatControllerIBUS.h"
#include "BoatControllerSBUS.h"
#include "BoatControllerPWM.h"
#include "BoatControllerGPS.h"
#include "BoatControllerCompass.h"
#include "BoatControllerConfig.h"

BoatControllerWififClass bContWifi;
BoatControllerServosClass bContServos;
BoatControllerTFTClass bContTFT;
BoatControllerSBUSClass bContSBus;
BoatControllerIBUSClass bContIBus;
BoatControllerPWMClass bContIPWM;
BoatControllerGPSClass bContGPS;
BoatControllerCompassClass bContCompass;
BoatControllerConfigClass bContConfig;

bool debugServos = false;
bool debugRCChannels = false;
bool debugCompass= false;
bool debugTracking;

float Pi = 3.14159;
float CompassHeading;

unsigned long bcCurrentMillis;
unsigned long bcLastCompassMillis;
unsigned long bcLastGPSMillis;
unsigned long NextLED = LOW;
bool TFTConnected = true;
DynamicJsonDocument config(32768);
JsonArray Servos;
JsonArray Channels;
JsonArray ServoTypes;

int RCType = 1;
File debugFile;
int debugWrites = 0;

void RCDoWork(void* pvParameters) {
	Serial.printf("RC Type is %d\n", RCType);

	switch (RCType) {
	case 1: // PPM
		bContIPWM.init();
		break;
	case 2: // SBus
		bContSBus.init();
		break;
	case 3: // IBus
		bContIBus.init();
		break;
	}

	for (;;) {
		// If the RC Type is PWM or IBus, get the readings here as its fast
		if (RCType == 1)
			bContIPWM.doWork();
		if (RCType == 2)
			bContSBus.doWork();
		if (RCType == 3)
			bContIBus.doWork();
		vTaskDelay(10);
	}
}

void WifiDoWork(void* pvParameters) {
    bContWifi.init();

    for (;;) {
        bContWifi.doWork();
		vTaskDelay(20);
    }
}

void TFTDoWork(void* pvParameters) {
    bContTFT.init();

    for (;;) {
        bContTFT.doWork();
        vTaskDelay(10);
    }
}

void GPSTask(void* pvParameters) {
    bContGPS.init();

    for (;;) {
        bContGPS.doWork();
        vTaskDelay(5000);

    }
}

void WriteDebug(String msg) {
	Serial.println(msg);
	bContTFT.TFTBuffer(msg);	
	char buf[200];
	sprintf(buf, "{\"debug\":\"%s\"}", msg.c_str());
	bContWifi.sendSocketMessage(buf);   


	if (debugFile) {
		debugFile.print(msg);
		debugWrites++;
		if (debugWrites > 10) {
			debugFile.close();
			debugFile = SD.open("/debug.txt", FILE_APPEND);
			debugWrites = 0;
		}
	}	
}

void setup() {
    Serial.begin(115200);   
    Serial.printf("Firmware Version %s\n", FirmwareVersion);
	
    if (!FFat.begin(FORMAT_SPIFFS_IF_FAILED)) {
		Serial.println("FFat Mount Failed");
		return;
    }
    
    if (!SD.begin()) {
		Serial.println("Card Mount Failed");
		TFTConnected = false;
	}else{
		//If the SD Card mounts and there is a config file on it - Copy it accross
		File conf = SD.open("/config.json", "r");
		if (conf) {
			Serial.print("Restoring Config file from SD-Card.");
			File tconf = FFat.open("/config.json", "w");
			while (conf.available()) {
				tconf.write(conf.read());
				Serial.print(".");
			}
			Serial.println("Done!");
			tconf.close();
			conf.close();
        }
		debugFile = SD.open("/debug.txt", "w");
    
	}

    bContConfig.init();
	bContConfig.readConfig(); 
	RCType = config["Params"]["RCType"];

	pinMode(LED_1, OUTPUT);
	pinMode(LED_2, OUTPUT);   


	bContCompass.init();	
	bContServos.init();	
	bContGPS.init();
	
	xTaskCreate(
		RCDoWork,	 // Function that should be called
		"RCDoWork", // Name of the task (for debugging)
		4096,		 // Stack size (bytes)
		NULL,		 // Parameter to pass
		5,			 // Task priority
		NULL		 // Task handle
	);
	

    if (TFTConnected) {
        xTaskCreate(
            TFTDoWork,    // Function that should be called
            "TFTDoWork",   // Name of the task (for debugging)
            4096,            // Stack size (bytes)
            NULL,            // Parameter to pass
            10,               // Task priority
            NULL             // Task handle
        );
    }

    
    xTaskCreate(
        WifiDoWork,    // Function that should be called
        "WifiDoWork",   // Name of the task (for debugging)
        4096,            // Stack size (bytes)
        NULL,            // Parameter to pass
        5,               // Task priority
        NULL             // Task handle
    );
   
}


void loop() {  
	bcCurrentMillis = millis();
	
	bContServos.doWork();
	
	if (bcCurrentMillis - bcLastCompassMillis > 1000) { // checks every 1 seconds
		digitalWrite(LED_2, NextLED);
		bcLastCompassMillis = bcCurrentMillis;
		bContCompass.doWork();
		if (NextLED == HIGH)
			NextLED = LOW;
		else
			NextLED = HIGH;
	}
	if (bcCurrentMillis - bcLastGPSMillis > 30000) { // checks every 30 seconds
		bcLastGPSMillis = bcCurrentMillis;
		bContGPS.doWork();
	}
	
	vTaskDelay(10);
}
