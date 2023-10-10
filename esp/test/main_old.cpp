//#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>

SoftwareSerial gps_serial_at(14, 15); // D7 , D8
TinyGPSPlus gps;
unsigned long gps_check_time_empty;
unsigned long gps_check_time_publish;

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

String gps_json_data;

boolean get_gps_data()
{
  while (gps_serial_at.available() > 0)
  {
    Serial.write(gps_serial_at.read());
    if (gps.encode(gps_serial_at.read()) && gps.location.isValid() && gps.time.isValid())
    {
      StaticJsonDocument<256> doc;
      doc["datetime"] = String(gps.date.year()) + "-" + add_zero(String(gps.date.month())) + "-" + add_zero(String(gps.date.day())) + " " + add_zero(String(gps.time.hour())) + ":" + add_zero(String(gps.time.minute())) + ":" + add_zero(String(gps.time.second()));
      doc["latitude"] = gps.location.lat();
      doc["longitude"] = gps.location.lng();
      doc["satellites"] = gps.satellites.value();
      doc["altitude"] = gps.altitude.meters();
      doc["speed"] = gps.speed.knots();
      doc["course"] = gps.course.deg();
      doc["hdop"] = gps.hdop.value();
      serializeJson(doc, gps_json_data);
      Serial.println(gps_json_data);
      gps_check_time_empty = millis();
      return true;
    }
    else
    {
      if ((millis() - gps_check_time_empty) >= 10000)
      {
        Serial.print(gps.encode(gps_serial_at.read()));
        Serial.print(gps.location.isValid());
        Serial.println(gps.time.isValid());
        Serial.println(gps.satellites.value());
        Serial.println(millis());
        Serial.println("no gps data");
        gps_check_time_empty = millis();
      }
      return false;
    }
  }
  return false;
}

void setup()
{
  //ESP.wdtDisable();    // disable software wdt to prevent resets
  //ESP.wdtEnable(8000); // enable software wdt and set time to 8 second to prevent resets
  Serial.begin(115200);
  Serial.println("Wait...");
  gps_serial_at.begin(9600);
  delay(1000);
  Serial.println("setup succeded");
  gps_check_time_publish = millis();
}

void loop()
{
 // ESP.wdtFeed();
  
  if ((millis() - gps_check_time_publish) >= 5000)
  {
    get_gps_data();
    gps_check_time_publish = millis();
  }
}
