#include "BoatControllerCompass.h"
#include "BoatControllerConfig.h"
#include "BoatControllerDefines.h"
#include "BoatControllerWifi.h"
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LIS2MDL.h>
#include <ArduinoJson.h>
#include "FFat.h"

Adafruit_LIS2MDL mag = Adafruit_LIS2MDL(12345);

extern float Pi;
extern float CompassHeading;
extern BoatControllerWififClass bContWifi;
extern BoatControllerConfigClass bContConfig;
extern DynamicJsonDocument config;
extern JsonArray Servos;

float MagMinX;
float MagMaxX;
float MagMinY;
float MagMaxY;
float MagMinZ;
float MagMaxZ;
float localOffset;
bool backupConfig = false;
unsigned long compCurrentMillis;
unsigned long compCompassBackupMillis;

void BoatControllerCompassClass::init()
{
    /* Initialise the sensor */
    if (!mag.begin())
    {
        /* There was a problem detecting the Compass ... check your connections */
        Serial.println("Ooops, no Compass detected ... Check your wiring!");        
    }
    else {
        Serial.println("Magnetometer initialised!");
		
		//Read config values from config file
		localOffset = config["Comp"]["Offset"];
		MagMinX = config["Comp"]["MMinX"];
		MagMaxX = config["Comp"]["MMaxX"];
		MagMinY = config["Comp"]["MMinY"];
		MagMaxY = config["Comp"]["MMaxY"];
		MagMinZ = config["Comp"]["MMinZ"];
		MagMaxZ = config["Comp"]["MMaxZ"];

		Serial.printf("Compass bias read from config - MinX=%.0f MaxX=%.0f:MinY=%.0f MaxY=%.0f:MinZ=%.0f MaxZ=%.0f\n", MagMinX, MagMaxX, MagMinY, MagMaxY, MagMinZ, MagMaxZ);
    }
}

void BoatControllerCompassClass::doWork()
{
    /* Get a new sensor event */
    sensors_event_t event;
    mag.getEvent(&event);
	compCurrentMillis = millis();

    bool updateCompassConfig = false;
    //Adjust compass calibration if necessary
	if (event.magnetic.x < MagMinX) {
		MagMinX = event.magnetic.x;
		updateCompassConfig = true;
	}
	if (event.magnetic.x > MagMaxX) {
		MagMaxX = event.magnetic.x;
		updateCompassConfig = true;
	}
	if (event.magnetic.y < MagMinY) {
		MagMinY = event.magnetic.y;
		updateCompassConfig = true;
	}
	if (event.magnetic.y > MagMaxY) {
		MagMaxY = event.magnetic.y;
		updateCompassConfig = true;
	}
	if (event.magnetic.z < MagMinZ) {
		MagMinZ = event.magnetic.z;
		updateCompassConfig = true;
	}
	if (event.magnetic.z > MagMaxZ) {
		MagMaxZ = event.magnetic.z;
		updateCompassConfig = true;
	}

	//If the bias endpoints are different update config with new values
	if (updateCompassConfig) {
		Serial.println("Updating Bias Values... ");

		config["Comp"]["MMinX"].set((int)MagMinX - 1);
		config["Comp"]["MMaxX"].set((int)MagMaxX + 1);
		config["Comp"]["MMinY"].set((int)MagMinY - 1);
		config["Comp"]["MMaxY"].set((int)MagMaxY + 1);
		config["Comp"]["MMinZ"].set((int)MagMinZ - 1);
		config["Comp"]["MMaxZ"].set((int)MagMaxZ + 1);

		compCompassBackupMillis = compCurrentMillis;
		backupConfig = true;
	}

	//Calibrate reading with bias values
    float hardiron_x = (MagMaxX + MagMinX) / 2;
    float hardiron_y = (MagMaxY + MagMinY) / 2;
    float hardiron_z = (MagMaxZ + MagMinZ) / 2;

    // Calculate the angle of the vector y,x
	float newCompassHeading = localOffset + ((atan2((event.magnetic.y - hardiron_y), (event.magnetic.x - hardiron_x)) * 180) / Pi);

    // Normalize to 0-360 and add a trimming offset from config if required
	if (newCompassHeading < 0)
    {
		newCompassHeading = 360 + newCompassHeading;
    }
	
    //Filter the compass sensitivity, If its less than 2 degrees, the servos will not move anyway
    if (abs(CompassHeading - newCompassHeading) > 2) {
		CompassHeading = newCompassHeading;
		
		//Adjust any compass based servos
		for (JsonVariant value : Servos) {
			if (value["type"].as<int>()==3) {
				
				int delta = value["homPos"].as<int>() - CompassHeading;
				if (delta > 180)
					delta = -1*CompassHeading;
				int newTargetPos = value["homPos"].as<int>() - delta;
				if (newTargetPos < value["min"].as<int>() || newTargetPos > value["max"].as<int>())
					newTargetPos = value["batPos"].as<int>();
				value["target"].set(newTargetPos);				
				
				Serial.printf("RF Movement: Compass=%.0f Target:%.0f NewTarget:%d\n", newCompassHeading, value["target"].as<float>(), newTargetPos);
				
				//Adjust any child servos attached to the Rangefinder type
				for (JsonVariant children : Servos) {
					if (children["parent"].as<int>() == value["ID"].as<int>()) {
						int TargetPos = newTargetPos - delta;
						if (TargetPos < children["min"].as<int>() || TargetPos > children["max"].as<int>())
							TargetPos = children["batPos"].as<int>();
						children["target"].set(TargetPos);						
					}
				}	
			}
		}

		//If the web page is attached send new bearing information for display
		char output[50];
        sprintf(output,"{\"sensor\":\"compassDiscImg\",\"bearing\":%.0f}", CompassHeading);
		bContWifi.sendSocketMessage(output);   

		//Backup the config if changes have been made to compass bias
		if (backupConfig && (compCurrentMillis > compCompassBackupMillis + 60000)) {
			bContConfig.saveConfig();
			backupConfig = false;			
		}

    }   
    
}
