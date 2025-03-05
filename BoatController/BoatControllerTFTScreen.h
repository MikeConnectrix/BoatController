// BoatControllerTFTScreen.h

#ifndef _BOATCONTROLLERTFTSCREEN_h
#define _BOATCONTROLLERTFTSCREEN_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class BoatControllerTFTScreenClass
{
 protected:
	

 public:
	void init();
	void draw();
	void doWork();
	
};

extern BoatControllerTFTScreenClass BoatControllerTFTScreen;

#endif

