#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
//#include <StreamDebugger.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>

#define SerialMon Serial
//StreamDebugger debugger(SerialAT, SerialMon);
SoftwareSerial gps_serial_at(14, 12); // D7 , D8
//StreamDebugger gps_debugger(gps_serial_at, SerialMon);
TinyGPSPlus gps;


// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
// #define TINY_GSM_YIELD() { delay(2); }

auto add_zero = [](String a) {
        if (a.length()==1)
        {
          return "0"+a;
        }
          else{return a;}
      };
String gps_json_data;

boolean get_gps_data()
{
  while (gps_serial_at.available()>0) 
  {
    if (gps.encode(gps_serial_at.read()) && gps.location.isValid() && gps.time.isValid()) 
    {
      StaticJsonDocument<256> doc;
      doc["datetime"] = String(gps.date.year())+"-"+add_zero(String(gps.date.month()))+"-"+add_zero(String(gps.date.day()))+" "+add_zero(String(gps.time.hour()))+":"+add_zero(String(gps.time.minute()))+":"+add_zero(String(gps.time.second()));
      doc["latitude"] = gps.location.lat();
      doc["longitude"] = gps.location.lng();
      doc["satellites"] = gps.satellites.value();
      doc["altitude"] = gps.altitude.meters();
      doc["speed"] = gps.speed.knots();
      doc["course"] = gps.course.deg();
      doc["hdop"] = gps.hdop.value();
      serializeJson(doc, gps_json_data);
      //SerialMon.println(gps_json_data);
      return true;
    }
    else
    {
      SerialMon.println(gps.encode(gps_serial_at.read()));
      SerialMon.println(gps.location.isValid());
      SerialMon.println(gps.time.isValid());
      SerialMon.println(gps.satellites.value());
      SerialMon.println(millis());
      SerialMon.println("no gps data");
      return false;
    }
  }
  return false;
}



void setup() {
  ESP.wdtDisable(); //disable software wdt to prevent resets
  ESP.wdtEnable(8000); //enable software wdt and set time to 8 second to prevent resets
  SerialMon.begin(115200);
  SerialMon.println("Wait...");
  gps_serial_at.begin(9600);
  delay(1000);
  SerialMon.println("setup succeded");
}

void loop() {
ESP.wdtFeed();
  get_gps_data();

}

