#include "BoatControllerServos.h"
#include "Wire.h"
#include "Adafruit_PWMServoDriver.h"
#include "BoatControllerDefines.h"
#include <ArduinoJson.h>


const int PWMFreq = 1000; /* 1 KHz */
const int PWMResolution = 8;
const int PWMSpeedChannel = 4;
extern DynamicJsonDocument config;
extern void WriteDebug(String msg);
extern bool debugServos;
extern bool debugTracking;
extern bool debugRCChannels;
extern JsonArray Channels;

	//Create Adafruit_PWMServoDriver object: servoBoard
Adafruit_PWMServoDriver servoBoard[16];

unsigned long currentMillis;  //stores current board time in milliseconds
unsigned long resetMillis;    // keeps track of next time to check if servos have finished moving
int resetTimer = 5000;        //checks every 2 seconds
int AttachedBoards = 0;
extern JsonArray Servos;
extern JsonArray ServoTypes;
extern float CompassHeading;

int dir;
bool alreadyRunning = false;

void BoatControllerServosClass::init()
{
	Wire.begin();
	//Scan I2C Bus to list any extra boards
	scanI2C();
	
	currentMillis = millis();

	JsonArray Controllers = config["Cont"];
	Servos = config["Servo"];
	
	if (Controllers) {
		// Set up controller boards from config
		for (JsonVariant value : Controllers) {
			servoBoard[AttachedBoards] = Adafruit_PWMServoDriver(value["I2C"].as<uint8_t>());
			servoBoard[AttachedBoards].begin();
			servoBoard[AttachedBoards].setPWMFreq(SERVO_FREQ); // Analog servos run at ~50 Hz updates
			Serial.printf("Attached %s controller at address %d\n", value["dscn"].as<const char*>(), value["I2C"].as<uint8_t>());
			AttachedBoards++;
		}
	}
	else
		Serial.println("Error: No controller configurations found!");
	
	if (Servos) {
		// Initialise servos with default positions
		if (debugServos) {
			char buf[100];
			sprintf(buf, "Initalising servos (%d found)\n", Servos.size());
			WriteDebug(buf);
		}

		for (JsonVariant value : Servos) {
			value["target"].set(ServoTypes[value["type"].as<int>()]["homPos"].as<int>());
			value["current"].set(value["target"].as<int>());
			value["millis"].set(currentMillis);
		}
	}
	else
		Serial.println("Error: No servo configurations found!");
}

//this function deals with moving all the servos at the correct speed.
void servoControl() {    
	int sendPos;
	bool ServoMoved = false;
	for (JsonVariant value : Servos) {			
		JsonVariant sType = ServoTypes[value["type"].as<int>()];
		int servoSpeed = value["spd"].as<int>();
		if (servoSpeed == 0)
			servoSpeed = sType["spd"].as<int>();
		int StepSize = value["step"].as<int>();
		if (StepSize == 0)
			StepSize = sType["step"].as<int>();
		

		if (currentMillis - value["millis"].as<int>() >= servoSpeed) {			
			if (abs(value["target"].as<int>() - value["current"].as<int>()) > StepSize) {
								
				if (value["target"].as<int>() > sType["max"].as<int>())
					value["target"].set(sType["max"].as<int>());
				if (value["target"].as<int>() < sType["min"].as<int>())
					value["target"].set(sType["min"].as<int>());

				int current = value["current"].as<int>();
				int target = current;
				if (current < value["target"].as<int>())
					target += StepSize;
				else
					target -= StepSize;

				if (value["rev"].as<String>().equals("true"))
					sendPos = map(sType["max"].as<int>()-target, sType["min"].as<int>(), sType["max"].as<int>(), USMIN, USMAX);
				else
					sendPos = map(target, sType["min"].as<int>(), sType["max"].as<int>(), USMIN, USMAX);

				int ControllerID = value["Ctrl"].as<int>();
				
				//Error trap that controllers have successfully set up
				if (AttachedBoards>0 && ControllerID <= AttachedBoards) {
					//servoBoard[ControllerID].writeMicroseconds(value["Prt"].as<int>(), sendPos);
					servoBoard[ControllerID].setPWM(value["Prt"].as<int>(), 0, sendPos);
					ServoMoved = true;
					vTaskDelay(10);
					if (debugServos) {
						char buf[200];
						sprintf(buf, "Moving %s from %d to %d speed=%d", value["dscn"].as<String>().c_str(), current, target, servoSpeed);
						WriteDebug(buf);
					}
				}
				
				value["current"].set(target);
				value["millis"].set(currentMillis);

				
				for (JsonVariant childServo : Servos) {
					if (childServo["cc"].as<String>().equals(value["ID"].as<String>()))
						childServo["target"].set(target);
				}
				if (value["type"].as<int>() == 3)
					value["homPos"].set(target);
				
			}
		}

		if (!ServoMoved && value["track"].as<String>().equals("true") && abs(value["target"].as<int>() - value["current"].as<int>()) < 5 && abs(value["current"].as<int>() - value["homePos"].as<int>()) > 20 && !value["tracking"].as<String>().equals("true")) {
			value["tracking"].set("true");
			int TargetBearing = CompassHeading - (value["current"].as<int>() - ((sType["min"].as<int>() + sType["max"].as<int>()) / 2));
			if (TargetBearing > 360)
				TargetBearing -= 360;
			if (TargetBearing <0)
				TargetBearing += 360;

			value["tBear"].set(TargetBearing);
			if (debugTracking) {
				char buf[200];
				sprintf(buf, "Compass tracking activated %d Tar=%d Cur=%d  Bear=%d", value["ID"].as<int>(), value["target"].as<int>(), value["current"].as<int>(), TargetBearing);
				WriteDebug(buf);
			}
			
		}
	}
}

void BoatControllerServosClass::doWork()
{
    if (!alreadyRunning) {
        alreadyRunning = true;
        currentMillis = millis();
        if (AttachedBoards>0) servoControl();  //servo control function        
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

void BoatControllerServosClass::moveServo(int channelID,int ServoID, float newValue) {
	JsonVariant servo = Servos[ServoID];
	JsonVariant value = Channels[channelID];
	bool servoMoved = false;

	int newTarget = 0;

	if (servo["abs"].as<String>().equals("true")) {
		if (newValue < value["min"].as<int>())
			newValue = value["min"].as<int>();
		if (newValue > value["max"].as<int>())
			newValue = value["max"].as<int>();
			JsonVariant servo = Servos[value["servo"]];
		newTarget = map(newValue, value["min"].as<int>(), value["max"].as<int>(), ServoTypes[servo["type"].as<int>()]["min"].as<int>(), ServoTypes[servo["type"].as<int>()]["max"].as<int>());


			if (newTarget > ServoTypes[servo["type"].as<int>()]["max"].as<int>())
				newTarget = ServoTypes[servo["type"].as<int>()]["max"].as<int>();
			if (newTarget < ServoTypes[servo["type"].as<int>()]["min"].as<int>())
				newTarget = ServoTypes[servo["type"].as<int>()]["min"].as<int>();

			if (servo["target"].as<int>() != newTarget) {
				servo["target"].set(newTarget);
				servoMoved = true;
			}
		}	
	else {
		int ServoCurrentPos = servo["current"].as<int>();

		if (newValue - 5 > 0) {
			if (ServoCurrentPos + 2 < ServoTypes[servo["type"].as<int>()]["max"].as<int>())
				ServoCurrentPos += 2;
		}

		if (newValue + 5 < 0) {
			if (ServoCurrentPos - 2 > ServoTypes[servo["type"].as<int>()]["min"].as<int>())
				ServoCurrentPos -= 2;
		}

		if (ServoCurrentPos != servo["target"].as<int>()) {
			Serial.printf("Reading Channel %d\n", channelID);

			newTarget = ServoCurrentPos;
			if (newTarget > ServoTypes[servo["type"].as<int>()]["max"].as<int>())
				newTarget = ServoTypes[servo["type"].as<int>()]["max"].as<int>();
			if (newTarget < ServoTypes[servo["type"].as<int>()]["min"].as<int>())
				newTarget = ServoTypes[servo["type"].as<int>()]["min"].as<int>();
			if (servo["target"].as<int>() != newTarget) {
				servo["target"].set(newTarget);
				servoMoved = true;
			}
		}
	}

	if (servoMoved && servo["tracking"].as<String>().equals("true")) {
		servo["tracking"].clear();
		if (debugTracking) {
			char buf[100];
			sprintf(buf, "Compass tracking deactivated %d", servo["ID"].as<int>());
			WriteDebug(buf);
		}
	}

	if (servoMoved && debugRCChannels) {
		char buf[200];
		sprintf(buf, "Servo %s : %d", servo["dscn"].as<String>().c_str(), servo["target"].as<int>());
		WriteDebug(buf);
	}
}

