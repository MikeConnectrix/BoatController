// BoatControllerPWM.h

#ifndef _BOATCONTROLLERPWM_h
#define _BOATCONTROLLERPWM_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
class BoatControllerPWMClass {
protected:
public:
	void init();
	void doWork();

private:
};

#endif

