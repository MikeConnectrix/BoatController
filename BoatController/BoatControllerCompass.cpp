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
extern JsonArray ServoTypes;
extern void WriteDebug(String msg);
extern bool debugCompass;
extern bool debugTracking;

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
		WriteDebug("Ooops, no Compass detected ... Check your wiring!\n");        
    }
    else {
		WriteDebug("Magnetometer initialised!\n");
		
		//Read config values from config file
		localOffset = config["Comp"]["Offset"];
		MagMinX = config["Comp"]["MMinX"];
		MagMaxX = config["Comp"]["MMaxX"];
		MagMinY = config["Comp"]["MMinY"];
		MagMaxY = config["Comp"]["MMaxY"];
		MagMinZ = config["Comp"]["MMinZ"];
		MagMaxZ = config["Comp"]["MMaxZ"];

		if (debugCompass) {
			char buf[100];
			sprintf(buf, "Compass bias read from config - MinX=%.0f MaxX=%.0f:MinY=%.0f MaxY=%.0f:MinZ=%.0f MaxZ=%.0f\n", MagMinX, MagMaxX, MagMinY, MagMaxY, MagMinZ, MagMaxZ);
			WriteDebug(buf);
		}
    }
}

int GetHeadingdifference(int initial, int final) {
	int directionA = final - initial;
	int directionB = 360 - (final + initial);
	int difference = 0;

	if (abs(directionA) < abs(directionB)) {
		difference = directionA;
	}
	else {
		difference = directionB;
	}

	return difference;
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
		if (MagMinX < -200)
			MagMinX = 0;
		else
		updateCompassConfig = true;
	}
	if (event.magnetic.x > MagMaxX) {
		MagMaxX = event.magnetic.x;
		if (MagMaxX > 200)
			MagMaxX = 0;
		else
			updateCompassConfig = true;
	}
	if (event.magnetic.y < MagMinY) {
		MagMinY = event.magnetic.y;
		if (MagMinY < -200)
			MagMinY = 0;
		else
			updateCompassConfig = true;
	}
	if (event.magnetic.y > MagMaxY) {
		MagMaxY = event.magnetic.y;
		if (MagMaxY > 200)
			MagMaxY = 0;
		else
			updateCompassConfig = true;
	}
	if (event.magnetic.z < MagMinZ) {
		MagMinZ = event.magnetic.z;
		if (MagMinZ < -200)
			MagMinZ = 0;
		else
			updateCompassConfig = true;
	}
	if (event.magnetic.z > MagMaxZ) {
		MagMaxZ = event.magnetic.z;
		if (MagMaxZ > 200)
			MagMaxZ = 0;
		else
			updateCompassConfig = true;
	}

	//If the bias endpoints are different update config with new values
	if (updateCompassConfig) {
		WriteDebug("Updating Bias Values...");		
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
	
    //Filter the compass sensitivity, If its less than 5 degrees, the servos will not move anyway
    if (abs(CompassHeading - newCompassHeading) > 5) {
		CompassHeading = newCompassHeading;
	
		char output[200];
        
		//Adjust any compass based servos
		for (JsonVariant value : Servos) {
			if (value["track"].as<String>().equals("true")) {	
				if (value["tracking"].as<String>().equals("true")) {
					int delta = value["tBear"].as<int>();
					int current = value["target"].as<int>();
					int newTargetPos = current - GetHeadingdifference(CompassHeading, delta);
					JsonVariant sType = ServoTypes[value["type"].as<int>()];
					if (newTargetPos < sType["min"].as<int>() || newTargetPos > sType["max"].as<int>())
						newTargetPos = sType["batPos"].as<int>();
					if (debugTracking) {
						sprintf(output, "Target Bearing %d CPos=%d NPos=%d", delta, current, newTargetPos);
						WriteDebug(output);
					}
					value["target"].set(newTargetPos);
					
				}
			}
		}

		//If the web page is attached send new bearing information for display
		sprintf(output,"{\"sensor\":\"compassDiscImg\",\"bearing\":%.0f}", CompassHeading);
		bContWifi.sendSocketMessage(output); 
		if (debugCompass) {
			sprintf(output, "Compass Bearing %.0f", CompassHeading);
			WriteDebug(output);
		}

		//Backup the config if changes have been made to compass bias
		if (backupConfig && (compCurrentMillis > compCompassBackupMillis + 60000)) {
			bContConfig.saveConfig();
			backupConfig = false;				
		}

    }   
    
}
