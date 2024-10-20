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
String firmwareVersion = "2.4";
String firmwareUpdateFile = "BoatController25.bin";



float Pi = 3.14159;
float CompassHeading;

unsigned long bcCurrentMillis;
unsigned long bcLastCompassMillis;
unsigned long NextLED = LOW;
bool TFTConnected = true;
DynamicJsonDocument config(8192);
JsonArray Servos;
int RCType = 1;

void ServoDoWork(void* pvParameters) {
   
    bContServos.init();	
   
    for (;;) {
		//If the RC Type is PWM, get the readings here as its fast
		if (RCType==1) bContIPWM.doWork();
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

void SBusDoWork(void* pvParameters) {
	bContSBus.init();

	for (;;) {
		bContSBus.doWork();
		vTaskDelay(2);
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

void setup() {
    Serial.begin(115200);   
    Serial.printf("Firmware Version %s\n",firmwareVersion);
	
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
    
	}


    bContConfig.init();
	bContConfig.readConfig(); 
	RCType = config["Params"]["RCType"];
    bContCompass.init();
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);    

    
    xTaskCreate(
		ServoDoWork,   // Function that should be called
		"ServoDoWork", // Name of the task (for debugging)
		4092,		   // Stack size (bytes)
		NULL,		   // Parameter to pass
		10,			   // Task priority
		NULL		   // Task handle
	);

	switch (RCType) {
	case 1://PPM
		bContIPWM.init();
		break;
	case 2://SBus
		xTaskCreate(
			SBusDoWork,	  // Function that should be called
			"SBusDoWork", // Name of the task (for debugging)
			4096,		 // Stack size (bytes)
			NULL,		 // Parameter to pass
			20,			 // Task priority
			NULL		 // Task handle
		);
		break;
	case 3://IBus
		bContIBus.init();
		break;
	}
	

    if (TFTConnected) {
        xTaskCreate(
            TFTDoWork,    // Function that should be called
            "TFTDoWork",   // Name of the task (for debugging)
            4096,            // Stack size (bytes)
            NULL,            // Parameter to pass
            30,               // Task priority
            NULL             // Task handle
        );
    }

    
    xTaskCreate(
        WifiDoWork,    // Function that should be called
        "WifiDoWork",   // Name of the task (for debugging)
        4096,            // Stack size (bytes)
        NULL,            // Parameter to pass
        40,               // Task priority
        NULL             // Task handle
    );

    xTaskCreate(
        GPSTask,    // Function that should be called
        "GPSTask",   // Name of the task (for debugging)
        2048,            // Stack size (bytes)
        NULL,            // Parameter to pass
        50,               // Task priority
        NULL             // Task handle
    );

    
}


void loop() {  	
    vTaskDelay(10);
}
