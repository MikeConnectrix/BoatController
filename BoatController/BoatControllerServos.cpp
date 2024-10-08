#include "BoatControllerServos.h"
#include "Wire.h"
#include "Adafruit_PWMServoDriver.h"
#include "BoatControllerDefines.h"
#include <ArduinoJson.h>


const int PWMFreq = 1000; /* 1 KHz */
const int PWMResolution = 8;
const int PWMSpeedChannel = 4;
extern DynamicJsonDocument config;

//Create Adafruit_PWMServoDriver object: servoBoard
Adafruit_PWMServoDriver servoBoard[16];

unsigned long currentMillis;  //stores current board time in milliseconds
unsigned long resetMillis;    // keeps track of next time to check if servos have finished moving
int resetTimer = 5000;        //checks every 2 seconds
int AttachedBoards = 0;
extern JsonArray Servos;

int dir;
bool alreadyRunning = false;

void BoatControllerServosClass::init()
{
	Wire.begin();
	//Scan I2C Bus to list any extra boards
	scanI2C();
	
	currentMillis = millis();

	JsonArray Controllers = config["Controllers"];
	Servos = config["Servos"];
	
	//Set up controller boards from config
	for (JsonVariant value : Controllers) {
		servoBoard[AttachedBoards] = Adafruit_PWMServoDriver(value["I2CAddress"].as<uint8_t>());
		servoBoard[AttachedBoards].begin();
		servoBoard[AttachedBoards].setPWMFreq(SERVO_FREQ); // Analog servos run at ~50 Hz updates
		Serial.printf("Attached %s controller at address %d\n", value["description"].as<const char*>(), value["I2CAddress"].as<uint8_t>());
		AttachedBoards++;
	}

	//Initialise servos with default positions
	for (JsonVariant value : Servos) {
		value["target"].set(value["homePos"].as<int>());
		value["current"].set(0);
		value["millis"].set(currentMillis);
	}
}

//this function deals with moving all the servos at the correct speed.
void servoControl() {    
	int sendPos;
	for (JsonVariant value : Servos) {
		if (value["target"].as<int>() != value["current"].as<int>()) {
			if (currentMillis - value["millis"].as<int>() >= value["speed"].as<int>()) {
				value["millis"].set(currentMillis);
				if (value["target"].as<int>() > value["max"].as<int>())
					value["target"].set(value["max"].as<int>());
				
				int current = value["current"].as<int>();
				
				if (current < value["target"].as<int>()) {
					current++;
				}
				else
					current--;
				
				sendPos = map(current, 0, value["max"].as<int>(), USMIN, USMAX);
				servoBoard[value["Controller"].as<int>()].writeMicroseconds(value["Port"].as<int>(),sendPos);
				value["current"].set(current);				
				
			}
		}
	}
}

void BoatControllerServosClass::doWork()
{
    if (!alreadyRunning) {
        alreadyRunning = true;
        currentMillis = millis();
        servoControl();  //servo control function        
        alreadyRunning = false;
    }	

}

void BoatControllerServosClass::scanI2C() {

    byte error, address;
	int nDevices;
	Serial.println("Scanning...");
	nDevices = 0;
	for (address = 1; address < 127; address++) {
		Wire.beginTransmission(address);
		error = Wire.endTransmission();
		if (error == 0) {
			Serial.print("I2C device found at address ");
			if (address < 16) {
				Serial.print("0");
			}
			Serial.println(address);
			nDevices++;
		}
		else if (error == 4) {
			Serial.print("Unknow error at address 0x");
			if (address < 16) {
				Serial.print("0");
			}
			Serial.println(address);
		}
	}
	if (nDevices == 0) {
		Serial.println("No I2C devices found\n");
	}
	else {
		Serial.println("done\n");
	}
}

