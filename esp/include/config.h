#ifndef config_h
#define config_h

#include <WString.h>

#define USE_CONNECTION_GSM
#define RXGSM D7 // 13
#define TXGSM D8 // 15
#define PWRGSM D6
#define HLTGSM D3

String apn_name = "mtnirancell";
String gprs_username = "";
String gprs_password = "";
String sim_pin_cod = "21831454";

String broker_ip = "109.122.233.74"; //"akranaaa.ddns.net";
uint16_t broker_port = 1883;
String broker_client_id = "esp8266_1";
String broker_username = "admin";
String broker_password = "admin";
#define HLTMQTT D4

#define HAS_SENSOR_GPS
#define RXGPS D2 // 3
#define TXGPS D3 // 1
#define PWRGPS D1
#define HLTGPS D5
String gps_topic = "gps";
unsigned long gps_interval_time_publish = 5000;

#endif