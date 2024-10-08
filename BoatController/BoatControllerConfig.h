// BoatControllerConfig.h

#ifndef _BOATCONTROLLERCONFIG_h
#define _BOATCONTROLLERCONFIG_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#include <ArduinoJson.h>

class BoatControllerConfigClass {
protected:
public:
	void init();
	void readConfig();
	void saveConfig();
	JsonVariant getElement(String ElementName);

private:
};

#endif

