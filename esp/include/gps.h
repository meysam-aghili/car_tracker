#ifndef gps_h
#define gps_h

#include <TinyGPS++.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include "config.h"
#include "util.h"

SoftwareSerial gps_serial_at(RXGPS, TXGPS);
TinyGPSPlus gps_client;
unsigned long gps_check_time_publish;
bool gps_is_healthy = false;

void gps_set_health_state(bool state)
{
  gps_is_healthy = state;
  (state == true) ? digitalWrite(HLTGPS, HIGH) : digitalWrite(HLTGPS, LOW);
}

void gps_power_on()
{
  digitalWrite(PWRGPS,HIGH);  
}

void gps_power_off()
{
  digitalWrite(PWRGPS,LOW);  
}

void gps_hard_restart()
{
  gps_power_off();
  delay(100);
  gps_power_on();
}

bool gps_init()
{
  gps_serial_at.begin(9600);
  pinMode(PWRGPS ,OUTPUT);
  pinMode(HLTGPS, OUTPUT);
  digitalWrite(HLTGPS, LOW);
  gps_hard_restart();
  delay(50);
  gps_check_time_publish = millis();
  return true;
}

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
  gps_serial_at.listen();
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
    gps_serial_at.flush();
    gps_serial_at.stopListening();
    gps_set_health_state(true);
    return gps_json_data;
  }
  else
  {
    String res = "{'is_valid_encode':" + String(gps_client.encode(gps_serial_at.read())) + ",'is_valid_location':" + String(gps_client.location.isValid()) + ",'is_valid_time':" + String(gps_client.time.isValid()) + ",'satellites_value':" + String(gps_client.satellites.value()) + "}";
    gps_serial_at.flush();
    gps_serial_at.stopListening();
    gps_set_health_state(false);
    return res;
  }
}

#endif