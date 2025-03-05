// BoatControllerServos.h

#ifndef _BOATCONTROLLERSERVOS_h
#define _BOATCONTROLLERSERVOS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class BoatControllerServosClass
{
protected:
public:
	void init();
	void doWork();
	void scanI2C();
	void moveServo(int channelID,int ServoID, float newValue);

private:

};

#endif

