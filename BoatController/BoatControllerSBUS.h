// BoatControllerSBUS.h

#ifndef _BOATCONTROLLERSBUS_h
#define _BOATCONTROLLERSBUS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class BoatControllerSBUSClass
{
protected:
public:
	void init();
	void doWork();
private:
};

#endif

