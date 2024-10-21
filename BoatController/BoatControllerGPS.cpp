// 
// 
// 

#include "BoatControllerGPS.h"
#include "BoatControllerDefines.h"
#include <TinyGPS++.h>
#include <TinyGPSPlus.h>
#include <QMC5883LCompass.h>

QMC5883LCompass compass;
TinyGPSPlus gps;  // the TinyGPS++ object

uint32_t LastCompass = millis();


void BoatControllerGPSClass::init()
{
    Serial2.begin(GPS_BAUDRATE, SERIAL_8N1, RXD2, TXD2);

    compass.init();
    compass.setCalibrationOffsets(-312.00, -1030.00, -11.00);
    compass.setCalibrationScales(0.90, 1.17, 0.97);
}

void BoatControllerGPSClass::doWork()
{
    int x, y, z;

    uint16_t timenow = millis();

    //if (timenow > LastCompass + 5000)
    //{
    //    // Read compass values
    //    compass.read();

    //    // Return XYZ readings
    //    x = compass.getX();
    //    y = compass.getY();
    //    z = compass.getZ();

    //    float heading = atan2(y, x);
    //    if (heading < 0) {
    //        heading += 2 * PI;
    //    }

    //    Serial.print("Heading: ");
    //    Serial.println(heading * 180 / PI);
    //    LastCompass = timenow;
    //}

    /*if (Serial2.available() > 0) {
        if (gps.encode(Serial2.read())) {
            if (gps.location.isValid()) {
                Serial.print(F("- latitude: "));
                Serial.println(gps.location.lat(), 4);

                Serial.print(F("- longitude: "));
                Serial.println(gps.location.lng(), 4);

                Serial.print(F("- altitude: "));
                if (gps.altitude.isValid())
                    Serial.println(gps.altitude.meters());
                else
                    Serial.println(F("INVALID"));
            }
            else {
                Serial.println(F("- location: INVALID"));
            }

            Serial.print(F("- speed: "));
            if (gps.speed.isValid()) {
                Serial.print(gps.speed.kmph());
                Serial.println(F(" km/h"));
            }
            else {
                Serial.println(F("INVALID"));
            }

            Serial.print(F("- GPS date&time: "));
            if (gps.date.isValid() && gps.time.isValid()) {
                Serial.print(gps.date.year());
                Serial.print(F("-"));
                Serial.print(gps.date.month());
                Serial.print(F("-"));
                Serial.print(gps.date.day());
                Serial.print(F(" "));
                Serial.print(gps.time.hour());
                Serial.print(F(":"));
                Serial.print(gps.time.minute());
                Serial.print(F(":"));
                Serial.println(gps.time.second());
            }
            else {
                Serial.println(F("INVALID"));
            }

            Serial.println();
        }
    }

    if (millis() > 10000 && gps.charsProcessed() < 10)
        Serial.println(F("No GPS data received: check wiring"));

    */

}