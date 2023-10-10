/*
 * Rui Santos
 * Complete Project Details https://randomnerdtutorials.com
 */

#include <SoftwareSerial.h>

// The serial connection to the GPS module
SoftwareSerial ss(14, 15);
unsigned long gps_check_time_publish;

void setup()
{
    Serial.begin(115200);
    ss.begin(9600);
    gps_check_time_publish = millis();
}

void loop()
{

    if ((millis() - gps_check_time_publish) >= 5000)
    {
        for (unsigned long start = millis(); millis() - start < 1000;)
        {
            while (ss.available())
            {
                Serial.write(ss.read());
            }
        }
        gps_check_time_publish = millis();
    }
}