
//-----------------------------------------------------General
#include <ArduinoJson.h>
#include "util.h"
#include "config.h"

//-----------------------------------------------------Connection
#if defined(USE_CONNECTION_GSM)
#include "gsm.h"
#else
#include "wifi_manager.h"
String wifi_ssid = "tplink_";
String wifi_password = "Sh!@#123";
WiFiClient connection_client;
#endif

//-----------------------------------------------------MQTT
#include "mqtt.h"

//-----------------------------------------------------GPS
#if defined(HAS_SENSOR_GPS)
#include "gps.h"
void publish_gps_data(String gps_json_data)
{
  if (publish_to_mqtt_broker(gps_topic.c_str(), gps_json_data.c_str()))
  {
    log_print("gps data published to mqtt broker");
  }
  else
  {
    log_print("cant publish gps data to mqtt broker");
  }
}
#endif

//-----------------------------------------------------Main

bool main_init()
{
  Serial.begin(115200);
  delay(1000);
  return true;
}

void setup()
{
  (main_init() == true) ? log_print("success : initialized.") : log_print("error : init failed.");
  (gsm_init() == true) ? log_print("success : gsm initialized.") : log_print("error : gsm init failed.");
  (gsm_check_network() == true) ? log_print("success : gsm network connected.") : log_print("error : gsm network has failed to connect.");
  (gsm_check_gprs() == true) ? log_print("success : gprs connected.") : log_print("error : gprs has failed to connect.");
  (mqtt_init() == true) ? log_print("success : mqtt initialized.") : log_print("error : mqtt init failed.");
  #if defined(HAS_SENSOR_GPS)
  (gps_init() == true) ? log_print("success : gps initialized.") : log_print("error : gps init failed.");
  #endif
}

void loop()
{
  mqtt_client.loop();
  if (millis() - gps_check_time_publish > gps_interval_time_publish)
  {
    String gps_data = get_gps_data(); 
    if (gps_data.length() > 50)
    {
      publish_gps_data(gps_data);
      log_print(gps_data);
    }
    gps_check_time_publish = millis();
  }
}
