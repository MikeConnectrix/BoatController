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
		localOffset = config["Compass"]["Offset"];
		JsonArray axisBias = config["Compass"]["AxisBias"];
		
		//Read axis bias settings from config
		for (JsonVariant value : axisBias) {
			float newValue = value["value"].as<float>();
			
			//If the value is spurios reset it
			if (abs(newValue) > 100)
				newValue = 0;
			if (value["bias"].as<String>().equals("MagMinX"))
				MagMinX = newValue;
				
			if (value["bias"].as<String>().equals("MagMaxX"))
				MagMaxX = newValue;
				
			if (value["bias"].as<String>().equals("MagMinY")) 
				MagMinY = newValue;
				
			if (value["bias"].as<String>().equals("MagMaxY")) 
				MagMaxY = newValue;
				
			if (value["bias"].as<String>().equals("MagMinZ")) 
				MagMinZ = newValue;
				
			if (value["bias"].as<String>().equals("MagMaxZ")) 
				MagMaxZ = newValue;				
		}		

		Serial.printf("Compass bias read from config - MinX=%.0f MaxX=%.0f:MinY=%.0f MaxY=%.0f:MinZ=%.0f MaxZ=%.0f\n", MagMinX, MagMaxX, MagMinY, MagMaxY, MagMinZ, MagMaxZ);
    }
}

void BoatControllerCompassClass::doWork()
{
    /* Get a new sensor event */
    sensors_event_t event;
    mag.getEvent(&event);

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

		JsonArray axisBias = config["Compass"]["AxisBias"];		
		for (int i = 0; i < axisBias.size(); i++) {
			JsonVariant item = axisBias[i];
			if (item["bias"].as<String>().equals("MagMinX"))
				item["value"].set((int)MagMinX-1);
			if (item["bias"].as<String>().equals("MagMaxX"))
				item["value"].set((int)MagMaxX+1);
			if (item["bias"].as<String>().equals("MagMinY"))
				item["value"].set((int)MagMinY-1);
			if (item["bias"].as<String>().equals("MagMaxY"))
				item["value"].set((int)MagMaxY+1);
			if (item["bias"].as<String>().equals("MagMinZ"))
				item["value"].set((int)MagMinZ-1);
			if (item["bias"].as<String>().equals("MagMaxZ"))
				item["value"].set((int)MagMaxZ+1);	
		}	
		
		bContConfig.saveConfig();

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
		Serial.printf("Compass Heading: %.0f\n", CompassHeading);
		
		//Adjust any compass based servos
		for (JsonVariant value : Servos) {
			if (value["type"].as<int>()==3) {
				int delta = 360-CompassHeading;
				if (delta > 180)
					delta = -1*CompassHeading;
				value["target"].set(value["homePos"].as<int>() -delta);

				//Adjust any child servos attached to the Rangefinder type
				for (JsonVariant children : Servos) {
					if (children["parent"].as<String>() == value["description"].as<String>()) {
						children["target"].set(children["homePos"].as<int>() - delta);						
					}
				}	
			}
		}

		//If the web page is attached send new bearing information for display
		char output[50];
        sprintf(output,"{\"sensor\":\"compassDiscImg\",\"bearing\":%.0f}", CompassHeading);
		bContWifi.sendSocketMessage(output);   

    }   
    
}
