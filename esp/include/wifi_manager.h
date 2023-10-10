#ifndef wifi_manager_h
#define wifi_manager_h

#include <ESP8266WiFi.h>
//#include <WiFi.h>

class connection_wifi
{
public:
  String ssid;
  String password;
  connection_wifi(String wifi_ssid, String wifi_password)
  {
    this->ssid = wifi_ssid;
    this->password = wifi_password;
  }
  bool setup_connection()
  {
    WiFi.disconnect();
    Serial.println("no wifi network");
    Serial.print("connecting to ");
    Serial.print(this->ssid);
    WiFi.begin(this->ssid, this->password);
    unsigned long start_time = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start_time) <= 50000)
    {
      delay(500);
      Serial.print(".");
    }
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("");
      Serial.println("wifi connection attempt failed");
      delay(5000);
      return 0;
    }
    else
    {
      Serial.println("");
      Serial.println("wifi connected");
      Serial.print("ip address is: ");
      Serial.println(WiFi.localIP());
      delay(100);
      return 1;
    }
  }
  bool check_connection()
  {
    if (WiFi.status() != WL_CONNECTED)
    {
      return setup_connection();
    }
    else
    {
      return 1;
    }
  }
};

#endif