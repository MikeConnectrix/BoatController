// BoatControllerIBus.h

#ifndef _BOATCONTROLLERIBUS_h
#define _BOATCONTROLLERIBUS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class BoatControllerIBUSClass {
protected:
public:
	void init();
	void doWork();

private:
};

#endif

