// BoatControllerTFT.h

#ifndef _BOATCONTROLLERTFT_h
#define _BOATCONTROLLERTFT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class BoatControllerTFTClass
{
protected:
public:
	void init();
	void doWork();
	

private:
	long SlideshownDelay;
};

#endif

