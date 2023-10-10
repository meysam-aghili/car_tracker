#include <Arduino.h>
#include <SoftwareSerial.h>

// #define USE_CONNECTION_GSM
#if defined(USE_CONNECTION_GSM)
#define TINY_GSM_MODEM_SIM800
#include "gsm.h"
String apn_name = "mtnirancell";
String username = "";
String password = "";
String sim_pin_cod = "21831454";
SoftwareSerial gsm_serial(14, 12); // RX:D5, TX:D6
// StreamDebugger debugger(gsm_serial, Serial);
// TinyGsm        modem(debugger);
TinyGsm gsm(gsm_serial);
TinyGsmClient connection_client(gsm);
connection_gsm conn(apn_name, username, password, sim_pin_cod, &gsm);
#else
#include "wifi_manager.h"
String wifi_ssid = "tplink_";
String wifi_password = "Sh!@#123";
WiFiClient connection_client;
connection_wifi conn(wifi_ssid, wifi_password);
#endif

#include "mqtt.h"
String broker_ip = "192.168.1.102"; // "akranaa.ddns.net";
uint16_t broker_port = 1883;
String broker_client_id = "esp8266_1";
String broker_username = "admin";
String broker_password = "admin";
PubSubClient mqtt_client_(connection_client);
mqtt mqtt_conn(broker_ip, broker_port, broker_client_id, broker_username, broker_password, mqtt_client_);

#include "gps.h"
#define RXGPS 14
#define TXGPS 15
// HardwareSerial gps_serial(1);
SoftwareSerial gps_serial(RXGPS, TXGPS); // RX:D2, TX:D1
gps gps_client(gps_serial);
String gps_topic = "gps";
unsigned long gps_check_time_publish;

#define HIT_SENSOR_PIN 15
String hit_sensor_topic = "hit_sensor";
unsigned long hit_sensor_check_time_publish;

void publish_gps_data()
{
  if (gps_client.new_gps_data)
  {
    if (mqtt_conn.publish_to_mqtt_broker(gps_topic.c_str(), gps_client.gps_json_data.c_str()))
    {
      Serial.println("gps data published to mqtt broker");
    }
    else
    {
      Serial.println("cant publish gps data to mqtt broker");
    }
  }
}

IRAM_ATTR void hit_sensor()
{
  if (millis() - hit_sensor_check_time_publish > 1000)
  {
    Serial.println("hit detected");
    String data;
    DynamicJsonDocument doc(1024);
    doc["device_id"] = ESP.getChipId();
    serializeJson(doc, data);
    if (!mqtt_conn.publish_to_mqtt_broker(hit_sensor_topic.c_str(), data.c_str()))
    {
      Serial.println("cant publish hit_sensor data to mqtt broker");
    }
    hit_sensor_check_time_publish = millis();
  }
}

void setup()
{
  // ESP.wdtDisable();
  // ESP.wdtEnable(8000);
  Serial.begin(115200);
  Serial.println("initializing connection");
#if defined(USE_CONNECTION_GSM)
  gsm_serial.begin(9600);
#endif
  if (!conn.setup_connection())
  {
    Serial.println("setup connection failed");
    ESP.restart();
  }
  Serial.println("setup connection succeded");
  mqtt_conn.setup_mqtt_broker();
  gps_serial.begin(9600);
  gps_check_time_publish = millis();
  hit_sensor_check_time_publish = millis();
  Serial.println("setup succeded");
  attachInterrupt(digitalPinToInterrupt(HIT_SENSOR_PIN), hit_sensor, RISING);
}

void loop()
{
  // ESP.wdtFeed();
  mqtt_conn.mqtt_loop();
  gps_client.get_data();
  if (millis() - gps_check_time_publish > 5000)
  {
    publish_gps_data();
    gps_check_time_publish = millis();
  }
}
