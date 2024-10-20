#include "BoatControllerPWM.h"
#include <ArduinoJson.h>
#include "BoatControllerDefines.h"


extern JsonArray Servos;

int16_t pwm_data[4];
int16_t last_pwm_data[4];


volatile long PWM1StartTime = 0;
volatile long PWM1CurrentTime = 0;
volatile long PWM1Pulses = 0;
volatile long PWM2StartTime = 0;
volatile long PWM2CurrentTime = 0;
volatile long PWM2Pulses = 0;
volatile long PWM3StartTime = 0;
volatile long PWM3CurrentTime = 0;
volatile long PWM3Pulses = 0;
volatile long PWM4StartTime = 0;
volatile long PWM4CurrentTime = 0;
volatile long PWM4Pulses = 0;


void IRAM_ATTR ReadPWM1Input() {
	PWM1CurrentTime = micros();
	if (PWM1CurrentTime > PWM1StartTime) {
		PWM1Pulses = PWM1CurrentTime - PWM1StartTime;
		PWM1StartTime = PWM1CurrentTime;
	}
}
void IRAM_ATTR ReadPWM2Input() {
	PWM2CurrentTime = micros();
	if (PWM2CurrentTime > PWM2StartTime) {
		PWM2Pulses = PWM2CurrentTime - PWM2StartTime;
		PWM2StartTime = PWM2CurrentTime;
	}
}
void IRAM_ATTR ReadPWM3Input() {
	PWM3CurrentTime = micros();
	if (PWM3CurrentTime > PWM3StartTime) {
		PWM3Pulses = PWM3CurrentTime - PWM3StartTime;
		PWM3StartTime = PWM3CurrentTime;
	}
}
void IRAM_ATTR ReadPWM4Input() {
	PWM4CurrentTime = micros();
	if (PWM4CurrentTime > PWM4StartTime) {
		PWM4Pulses = PWM4CurrentTime - PWM4StartTime;
		PWM4StartTime = PWM4CurrentTime;
	}
}

void BoatControllerPWMClass::init() {
	Serial.println("Initialising PWM interface");
	pinMode(PWMPin1, INPUT_PULLUP);
	pinMode(PWMPin2, INPUT_PULLUP);
	pinMode(PWMPin3, INPUT_PULLUP);
	pinMode(PWMPin4, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(PWMPin1), ReadPWM1Input, CHANGE);
	attachInterrupt(digitalPinToInterrupt(PWMPin2), ReadPWM2Input, CHANGE);
	attachInterrupt(digitalPinToInterrupt(PWMPin3), ReadPWM3Input, CHANGE);
	attachInterrupt(digitalPinToInterrupt(PWMPin4), ReadPWM4Input, CHANGE);
 }

 void BoatControllerPWMClass::doWork() {
	 if (PWM1Pulses < 2500) {
		 pwm_data[0] = PWM1Pulses;
	 }
	 if (PWM2Pulses < 2500) {
		 pwm_data[1] = PWM2Pulses;
	 }
	 if (PWM3Pulses < 2500) {
		 pwm_data[2] = PWM3Pulses;
	 }
	 if (PWM4Pulses < 2500) {
		 pwm_data[3] = PWM4Pulses;
	 }

	 for (int i = 0; i < 4; i++) {
		 if (std::abs(last_pwm_data[i] - pwm_data[i]) > 20) {
			 for (JsonVariant value : Servos) {
				 if (value["RC"].as<int>() == i) {
					 int target = map(pwm_data[i], 0, 2500, value["min"].as<int>(), value["max"].as<int>());
					 value["target"].set(target);
				 }
			 }
		 }
		 last_pwm_data[i] = pwm_data[i];
	 }

	 
 }

