#include <Arduino.h>
#include <SoftwareSerial.h>

#define HAS_NETWORK
#define USE_CONNECTION_GSM
#define HAS_SENSOR_GPS
// #define HAS_SENSOR_MQ9
// #define HAS_SENSOR_HIT
// #define HAS_SENSOR_HIT2
// #define HAS_SENSOR_DHT
// #define HAS_SENSOR_MOTION
// #define HAS_SENSOR_RELAY
// #define HAS_SENSOR_BUZZER

//-----------------------------------------------------Connection
#if defined(HAS_NETWORK)
#if defined(USE_CONNECTION_GSM)
#define TINY_GSM_MODEM_SIM800
#include "gsm.h"
String apn_name = "mtnirancell";
String username = "";
String password = "";
String sim_pin_cod = "21831454";
SoftwareSerial gsm_serial(3, 1);
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
#endif

//-----------------------------------------------------MQTT
#if defined(HAS_NETWORK)
#include "mqtt.h"
String broker_ip = "192.168.1.102"; // "akranaa.ddns.net";
uint16_t broker_port = 1883;
String broker_client_id = "esp8266_1";
String broker_username = "admin";
String broker_password = "admin";
PubSubClient mqtt_client_(connection_client);
mqtt mqtt_conn(broker_ip, broker_port, broker_client_id, broker_username, broker_password, mqtt_client_);
#endif

//-----------------------------------------------------GPS
#if defined(HAS_SENSOR_GPS)
#include "TinyGPS++.h"
#include <ArduinoJson.h>
#define RXGPS D7 // 13
#define TXGPS D8 // 15
SoftwareSerial gps_serial_at(RXGPS, TXGPS);
TinyGPSPlus gps_client;
String gps_topic = "gps";
unsigned long gps_check_time_publish;
unsigned long gps_interval_time_publish = 5000;

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
    return "{'is_valid_encode':" + String(gps_client.encode(gps_serial_at.read())) + ",'is_valid_location':" + String(gps_client.location.isValid()) + ",'is_valid_time':" + String(gps_client.time.isValid()) + ",'satellites_value':" + String(gps_client.satellites.value()) + "}";
  }
}

#if defined(HAS_NETWORK)
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
#endif

//-----------------------------------------------------Relay
#if defined(HAS_SENSOR_RELAY)
#define RELAY_PIN D0 // 16  Active Low
#endif

//-----------------------------------------------------Buzzer
#if defined(HAS_SENSOR_BUZZER)
#define BUZZER_PIN D5 // 14  Active High
#endif

//-----------------------------------------------------Hit
#if defined(HAS_SENSOR_HIT)
#define HIT_SENSOR_PIN D4 // 2 Active Low
String hit_sensor_topic = "hit_sensor";
unsigned long hit_sensor_check_time_publish;

IRAM_ATTR void hit_sensor()
{
  if (millis() - hit_sensor_check_time_publish > 1000)
  {
    Serial.println("hit detected");
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    String data;
    DynamicJsonDocument doc(1024);
    doc["device_id"] = ESP.getChipId();
    doc["hit"] = 1;
    serializeJson(doc, data);
    // if (!mqtt_conn.publish_to_mqtt_broker(hit_sensor_topic.c_str(), data.c_str()))
    //{
    //   Serial.println("cant publish hit_sensor data to mqtt broker");
    // }
    hit_sensor_check_time_publish = millis();
  }
}
#endif

//-----------------------------------------------------HIT2
#if defined(HAS_SENSOR_HIT2)
#define HIT2_SENSOR_PIN D1 // 5 Active High
String hit2_sensor_topic = "hit2_sensor";
unsigned long hit2_sensor_check_time_publish;

IRAM_ATTR void hit2_sensor()
{
  if (millis() - hit2_sensor_check_time_publish > 1000)
  {
    Serial.println("hit2 detected");
    String data;
    DynamicJsonDocument doc(1024);
    doc["device_id"] = ESP.getChipId();
    doc["hit2"] = 1;
    serializeJson(doc, data);
    // if (!mqtt_conn.publish_to_mqtt_broker(hit2_sensor_topic.c_str(), data.c_str()))
    //{
    //   Serial.println("cant publish hit2_sensor data to mqtt broker");
    // }
    hit2_sensor_check_time_publish = millis();
  }
}
#endif

//-----------------------------------------------------MQ9
#if defined(HAS_SENSOR_MQ9)
#define GAS_SENSOR_PIN A0
String gas_sensor_topic = "gas_sensor";

void gas_sensor()
{
  int gas_value = 0;
  for (int i = 0; i < 10; i++)
  {
    gas_value += analogRead(GAS_SENSOR_PIN);
    delay(50);
  }
  gas_value = gas_value / 10;
  String data;
  DynamicJsonDocument doc(1024);
  doc["device_id"] = ESP.getChipId();
  doc["gas_value"] = gas_value;
  serializeJson(doc, data);
  Serial.println(data);
  // if (!mqtt_conn.publish_to_mqtt_broker(gas_sensor_topic.c_str(), data.c_str()))
  //{
  //   Serial.println("cant publish gas_sensor data to mqtt broker");
  // }
}
#endif

//-----------------------------------------------------Motion
#if defined(HAS_SENSOR_MOTION)
#define MOTION_SENSOR_PIN D2 // 4 Active High
String motion_sensor_topic = "motion_sensor";
unsigned long motion_sensor_check_time_publish;

IRAM_ATTR void motion_sensor()
{
  if (millis() - motion_sensor_check_time_publish > 1000)
  {
    Serial.println("motion detected");
    digitalWrite(RELAY_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    String data;
    DynamicJsonDocument doc(1024);
    doc["device_id"] = ESP.getChipId();
    doc["motion"] = 1;
    serializeJson(doc, data);
    // if (!mqtt_conn.publish_to_mqtt_broker(motion_sensor_topic.c_str(), data.c_str()))
    //{
    //   Serial.println("cant publish motion_sensor data to mqtt broker");
    // }
    motion_sensor_check_time_publish = millis();
  }
}
#endif

//-----------------------------------------------------DHT
#if defined(HAS_SENSOR_DHT)
#include "DHT.h"
#define DHT_PIN D6 // 12 Active High
#define DHTTYPE DHT22

DHT dht(DHT_PIN, DHTTYPE);
String dht_sensor_topic = "dht_sensor";

void dht_sensor()
{
  String data;
  DynamicJsonDocument doc(1024);
  doc["device_id"] = ESP.getChipId();
  doc["humidity_value"] = dht.readHumidity();
  doc["temperature_value"] = dht.readTemperature();
  // doc["computeHeatIndex"] = dht.computeHeatIndex();
  serializeJson(doc, data);
  Serial.println(data);
  // if (!mqtt_conn.publish_to_mqtt_broker(gas_sensor_topic.c_str(), data.c_str()))
  //{
  //   Serial.println("cant publish gas_sensor data to mqtt broker");
  // }
}
#endif

//------------------------------------------------------------------------------------------------------------------------
void setup()
{
  ESP.wdtDisable();
  ESP.wdtEnable(8000);
  Serial.begin(115200);

#if defined(HAS_NETWORK)
  Serial.println("initializing connection");
#if defined(USE_CONNECTION_GSM)
  gsm_serial.begin(115200);
#endif

  if (!conn.setup_connection())
  {
    Serial.println("setup connection failed");
    ESP.restart();
  }
  Serial.println("setup connection succeded");
  //mqtt_conn.setup_mqtt_broker();
#endif

#if defined(HAS_SENSOR_GPS)
  gps_serial_at.begin(9600);
  gps_check_time_publish = millis();
#endif
  /*
  hit_sensor_check_time_publish = millis();
  pinMode(HIT_SENSOR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(HIT_SENSOR_PIN), hit_sensor, RISING);
  pinMode(HIT2_SENSOR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(HIT2_SENSOR_PIN), hit2_sensor, RISING);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(MOTION_SENSOR_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR_PIN), motion_sensor, RISING);
  pinMode(BUZZER_PIN, OUTPUT);
  dht.begin();
  */
  Serial.println("setup succeded");
}

void loop()
{
  ESP.wdtFeed();
//#if defined(HAS_NETWORK)
 // mqtt_conn.mqtt_loop();
//#endif


  /*
  if (millis() - gps_check_time_publish > gps_interval_time_publish)
  {

    String gps_data = get_gps_data();
    Serial.println(gps_data);
    if (gps_data.length() > 50)
    {
      // publish_gps_data();
    }
    gps_check_time_publish = millis();
  }
  */

}
