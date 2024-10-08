// BoatControllerCompass.h

#ifndef _BOATCONTROLLERCOMPASS_h
#define _BOATCONTROLLERCOMPASS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
class BoatControllerCompassClass
{
protected:
public:
	void init();
	void doWork();
private:
};

#endif

