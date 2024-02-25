#include <SoftwareSerial.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>

SoftwareSerial gps_serial_at(D7, D8); // D7 , D8
TinyGPSPlus gps_client;

auto add_zero = [](String a)
{
  if (a.length() == 1)
  {
    return "0" + a;
  }
  else
  {
    return a;
  }
};

String get_gps_data()
{
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (gps_serial_at.available())
    {
      gps_client.encode(gps_serial_at.read());
    }
  }
  if (gps_client.location.isValid() == 1)
  {
    DynamicJsonDocument doc(1024);
    String gps_json_data = "";
    doc["datetime"] = String(gps_client.date.year()) + "-" + add_zero(String(gps_client.date.month())) + "-" + add_zero(String(gps_client.date.day())) + " " + add_zero(String(gps_client.time.hour())) + ":" + add_zero(String(gps_client.time.minute())) + ":" + add_zero(String(gps_client.time.second()));
    doc["latitude"] = gps_client.location.lat();
    doc["longitude"] = gps_client.location.lng();
    doc["satellites"] = gps_client.satellites.value();
    doc["altitude"] = gps_client.altitude.meters();
    doc["speed"] = gps_client.speed.knots();
    doc["course"] = gps_client.course.deg();
    doc["hdop"] = gps_client.hdop.value();
    serializeJson(doc, gps_json_data);
    return gps_json_data;
  }
  else
  {
    return "{'is_valid_encode':" + String(gps_client.encode(gps_serial_at.read())) 
    +",'is_valid_location':" + String(gps_client.location.isValid()) 
    +",'is_valid_time':" + String(gps_client.time.isValid()) 
    +",'satellites_value':" + String(gps_client.satellites.value()) + "}";
  }
}

unsigned long gps_check_time_publish;
unsigned long gps_interval_time_publish = 5000;

void setup()
{
  Serial.begin(115200);
  gps_serial_at.begin(9600);
  gps_check_time_publish = millis();
}
void loop()
{
  if (millis() - gps_check_time_publish > gps_interval_time_publish)
  {
  Serial.println(get_gps_data());
  gps_check_time_publish = millis();
  }
}
