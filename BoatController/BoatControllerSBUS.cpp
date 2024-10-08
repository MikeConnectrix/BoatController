#include "BoatControllerDefines.h"
#include "BoatControllerSBUS.h"
#include "sbus.h"

/* SbusRx object on Serial1 */
bfs::SbusRx sbus_rx(&Serial1);
int16_t sbus_data[bfs::SbusRx::NUM_CH()];
int16_t lastSBusData[bfs::SbusRx::NUM_CH()];

void BoatControllerSBUSClass::init()
{
	pinMode(SBUSRX, INPUT);
    sbus_rx.Begin(SBUSRX, SBUSTX);
}

void BoatControllerSBUSClass::doWork()
{
    if (sbus_rx.Read()) {
        /* Display the received data */
		for (int8_t i = 0; i < bfs::SbusRx::NUM_CH(); i++) {
            sbus_data[i] = sbus_rx.ch(i);
            if (std::abs(lastSBusData[i] - sbus_data[i])>5)            {
              lastSBusData[i]=sbus_data[i];
              //servoData[i][SERVOTARGET] = 270 * lastSBusData[i] / 2000;  
              //Serial.printf("Change [%d] from [%d] to [%d]\n", i, lastSBusData[i], servoData[i][SERVOTARGET]); 
              
            }
        }
    }
}
