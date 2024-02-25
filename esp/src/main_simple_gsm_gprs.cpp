#include <SoftwareSerial.h>
#include <StreamDebugger.h>
#include <ArduinoJson.h>

#define HAS_NETWORK
#define USE_CONNECTION_GSM

//-----------------------------------------------------Connection
#if defined(HAS_NETWORK)
#if defined(USE_CONNECTION_GSM)
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h> //#include <gsm.h>
String apn_name = "mtnirancell";
String gprs_username = "";
String gprs_password = "";
String sim_pin_cod = "21831454";
SoftwareSerial gsm_serial(D7, D8);
// StreamDebugger debugger(gsm_serial, Serial);
// TinyGsm        modem(debugger);
TinyGsm gsm(gsm_serial);
TinyGsmClient connection_client(gsm);
#else
#include "wifi_manager.h"
String wifi_ssid = "tplink_";
String wifi_password = "Sh!@#123";
WiFiClient connection_client;
#endif
#endif

//-----------------------------------------------------MQTT
#if defined(HAS_NETWORK)
#include "mqtt.h"
String broker_ip = "akranaaa.ddns.net";
uint16_t broker_port = 1883;
String broker_client_id = "esp8266_1";
String broker_username = "admin";
String broker_password = "admin";
PubSubClient mqtt_client_(connection_client);
mqtt mqtt_conn(broker_ip, broker_port, broker_client_id, broker_username, broker_password, mqtt_client_);

String gps_topic = "gps";

void publish_gps_data(String gps_json_data)
{
  if (mqtt_conn.publish_to_mqtt_broker(gps_topic.c_str(), gps_json_data.c_str()))
  {
    Serial.println("gps data published to mqtt broker");
  }
  else
  {
    Serial.println("cant publish gps data to mqtt broker");
  }
}
#endif

void log_print(String text)
{
  Serial.println(text);
}

bool gsm_check_network()
{
  if (!gsm.isNetworkConnected())
  {
    log_print("no network. connecting to modem network...");
    if (!gsm.waitForNetwork())
    {
      log_print("modem network failed");
      return false;
    }
    else
    {
      log_print("modem network connected");
    }
  }
  return true;
}

bool gsm_check_gprs()
{
  if (!gsm_check_network())
  {
    return false;
  }
  if (!gsm.isGprsConnected())
  {
    log_print("no gprs. connecting to " + apn_name);
    boolean status = gsm.gprsConnect(apn_name.c_str(), gprs_username.c_str(), gprs_password.c_str());
    if (!status)
    {
      log_print("gprs connection attempt failed");
      return false;
    }
    else
    {
      log_print("gprs connected");
    }
  }
  return true;
}

void gsm_init()
{
  gsm.init();
  log_print("modem info: " + gsm.getModemInfo());
  log_print("sim status: " + String(gsm.getSimStatus()));
  if (gsm.getSimStatus() == 3)
  {
    gsm.simUnlock(sim_pin_cod.c_str());
  }
  gsm_check_gprs();
}


unsigned long gps_check_time_publish;
unsigned long gps_interval_time_publish = 5000;

void setup()
{
  Serial.begin(115200);
  log_print("serial started");
  delay(1000);
  gsm_serial.begin(115200);
  log_print("gsm serial started");
  delay(1000);
  gsm_init();

  mqtt_conn.setup_mqtt_broker();

  gps_check_time_publish = millis();
}

void loop()
{
  mqtt_conn.mqtt_loop();
  
  if (millis() - gps_check_time_publish > gps_interval_time_publish)
  {

    String gps_data = "dataaaaaa";//get_gps_data();
    log_print(gps_data);
   // if (gps_data.length() > 50)
    //{
       publish_gps_data(gps_data);
    //}
    gps_check_time_publish = millis();
  }

}
