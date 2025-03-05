// BoatControllerTFTScreenInput.h

#ifndef _BOATCONTROLLERTFTSCREENINPUT_h
#define _BOATCONTROLLERTFTSCREENINPUT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include "BoatControllerTFTScreen.h"

class BoatControllerTFTScreenInputClass {
 protected:


 public:
	 void draw();
	 void doWork();
};

extern BoatControllerTFTScreenInputClass BoatControllerTFTScreenInput;

#endif

