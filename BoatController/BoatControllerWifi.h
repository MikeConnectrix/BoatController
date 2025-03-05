// BoatControllerWifi.h

#ifndef _BOATCONTROLLERWIFI_h
#define _BOATCONTROLLERWIFI_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
class BoatControllerWififClass
{
protected:
public:
	void init();
	void doWork();
	void UpdateDeviceWebPages();
	void sendSocketMessage(String msg);
	IPAddress IP;
	String ControllerIPAddress = "";
	bool bUpdateWebPages = false;

private:

};

#endif

