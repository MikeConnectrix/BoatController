// BoatControllerGPS.h

#ifndef _BOATCONTROLLERGPS_h
#define _BOATCONTROLLERGPS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
class BoatControllerGPSClass
{
protected:
public:
	void init();
	void doWork();
private:
};

#endif

