#ifndef gps_h
#define gps_h

#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
class gps
{
public:
    String gps_json_data;
    bool new_gps_data = false;
    Stream &gps_serial;
    explicit gps(Stream &gps_serial) : gps_serial(gps_serial)
    {
        this->gps_check_time_empty = millis();
    }
    unsigned long gps_check_time_empty;
    TinyGPSPlus gps_client;

    String add_zero(String text)
    {
        if (text.length() == 1)
        {
            return "0" + text;
        }
        else
        {
            return text;
        }
    }

    bool get_data()
    {
        unsigned long start = millis();
        gps_serial.flush();
        do
        {
            while (gps_serial.available() > 0)
            {
                if (gps_client.encode(gps_serial.read()) && this->gps_client.location.isValid() && this->gps_client.time.isValid())
                {
                    DynamicJsonDocument doc(1024);
                    doc["datetime"] = String(this->gps_client.date.year()) + "-" + add_zero(String(this->gps_client.date.month())) + "-" + add_zero(String(this->gps_client.date.day())) + " " + add_zero(String(this->gps_client.time.hour())) + ":" + add_zero(String(this->gps_client.time.minute())) + ":" + add_zero(String(this->gps_client.time.second()));
                    doc["latitude"] = this->gps_client.location.lat();
                    doc["longitude"] = this->gps_client.location.lng();
                    doc["satellites"] = this->gps_client.satellites.value();
                    doc["altitude"] = this->gps_client.altitude.meters();
                    doc["speed"] = this->gps_client.speed.knots();
                    doc["course"] = this->gps_client.course.deg();
                    doc["hdop"] = this->gps_client.hdop.value();
                    this->gps_json_data = "";
                    serializeJson(doc, this->gps_json_data);
                    this->gps_check_time_empty = millis();
                    this->new_gps_data = true;
                    //Serial.println(this->gps_json_data);
                    return true;
                }
                else
                {
                    if ((millis() - this->gps_check_time_empty) >= 10000)
                    {
                        Serial.print(this->gps_client.encode(gps_serial.read()));
                        Serial.print(this->gps_client.location.isValid());
                        Serial.println(this->gps_client.time.isValid());
                        Serial.println(this->gps_client.satellites.value());
                        Serial.println(millis());
                        Serial.println("no gps data");
                        this->gps_check_time_empty = millis();
                        this->new_gps_data = false;
                    }
                    return false;
                }
            }
        }
        while (millis() - start < 1000);
        return false;
    }
};
#endif