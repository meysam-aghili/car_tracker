#include <SoftwareSerial.h>
#include <StreamDebugger.h>
#include <ArduinoJson.h>

void log_print(String text)
{
  Serial.println(text);
}

#define USE_CONNECTION_GSM
#define HAS_SENSOR_GPS

//-----------------------------------------------------Connection
#if defined(USE_CONNECTION_GSM)
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h> //#include <gsm.h>
String apn_name = "mtnirancell";
String gprs_username = "";
String gprs_password = "";
String sim_pin_cod = "21831454";
#define RXGSM D7 // 13
#define TXGSM D8 // 15
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

void send_at(String cmd)
{
  log_print("send : " + cmd);
  gsm_serial.println(cmd);
  gsm_serial.flush();
}

int8_t wait_response(uint32_t timeout_ms, String &data)
{
  uint32_t startMillis = millis();
  data.reserve(64);
  do
  {
    while (gsm_serial.available())
    {
      int8_t a = gsm_serial.read();
      if (a <= 0)
        continue; // Skip 0x00 bytes, just in case
      data += static_cast<char>(a);
      if (data.endsWith("OK\r\n"))
      {
        log_print("ok");
        return 1;
      }
      else if (data.endsWith("ERROR\r\n"))
      {
        log_print("error");
        return 2;
      }
    }
  } while (millis() - startMillis < timeout_ms);
  log_print("error : not responsed");
  return 0;
}

int8_t wait_response(uint32_t timeout_ms)
{
  String data;
  return wait_response(timeout_ms, data);
}

int8_t wait_response()
{
  return wait_response(1000);
}

bool gsm_check_heartbeat()
{
  unsigned int at_counter = 0;
  while (at_counter < 10)
  {
    send_at("AT");
    if (wait_response(1000) == 1)
    {
      return true;
    }
    at_counter++;
  }
  return false;
}

bool gsm_init()
{
  gsm_serial.begin(115200);
  delay(50);
  if (gsm_check_heartbeat() == false)
  {
    return false;
  }

  send_at("ATE0");
  if (wait_response(10) != 1)
  {
    return false;
  }

  send_at("AT+CMEE=0"); // turn off error codes
  wait_response();

  send_at("AT+CLTS=1"); // Enable Local Time Stamp for getting network time
  if (wait_response(10000L) != 1)
  {
    return false;
  }

  //! this will reset module every time
  // send_at("AT+CBATCHK=1"); // Enable battery checks
  // wait_response();

  return true;
}

bool gsm_check_network(uint32_t timeout_ms = 60000L)
{
  if (gsm_check_heartbeat())
  {
    for (uint32_t start = millis(); millis() - start < timeout_ms;)
    {
      send_at("AT+CREG?");
      String data;
      if (wait_response(10, data) == 1)
      {
        int status_index = data.indexOf(",");
        String status = data.substring(status_index + 1, status_index + 2);
        log_print("network status: " + status);
        // 0 Not registered, the device is currently not searching for new operator.
        // 1 Registered to home network.
        // 2 Not registered, but the device is currently searching for a new operator.
        // 3 Registration denied.
        // 4 Unknown. For example, out of range.
        // 5 Registered, roaming. The device is registered on a foreign (national or international) network.
        if (status == "1" || status == "5")
        {
          return true;
        }
      }
      delay(2000L);
    }
  }
  return false;
}

bool gsm_gprs_disconnect() 
{
    // Shut the TCP/IP connection
    // CIPSHUT will close *all* open connections
    send_at("AT+CIPSHUT");
    if (wait_response(60000L) != 1) { return false; }

    send_at("AT+CGATT=0");  // Detach from GPRS
    if (wait_response(60000L) != 1) { return false; }

    return true;
  }

bool gsm_gprs_connect(String apn, String user = "", String pwd = "")
{
  gsm_gprs_disconnect();

  send_at("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  wait_response();

  send_at("AT+SAPBR=3,1,\"APN\",\"" + apn + '"');
  wait_response();

  if (user && user.length() > 0)
  {
    send_at("AT+SAPBR=3,1,\"USER\",\"" + user + '"');
    wait_response();
  }
  if (pwd && pwd.length() > 0)
  {
    send_at("AT+SAPBR=3,1,\"PWD\",\"" + pwd + '"');
    wait_response();
  }

  // Define the PDP context
  send_at("AT+CGDCONT=1,\"IP\",\"" + apn + '"');
  wait_response();

  // Activate the PDP context
  send_at("AT+CGACT=1,1");
  wait_response(60000L);

  // Open the definied GPRS bearer context
  send_at("AT+SAPBR=1,1");
  wait_response(85000L);
  // Query the GPRS bearer context status
  send_at("AT+SAPBR=2,1");
  if (wait_response(30000L) != 1)
  {
    return false;
  }

  // Attach to GPRS
  send_at("AT+CGATT=1");
  if (wait_response(60000L) != 1)
  {
    return false;
  }

  // Set to multi-IP
  send_at("AT+CIPMUX=1");
  if (wait_response() != 1)
  {
    return false;
  }

  // Put in "quick send" mode (thus no extra "Send OK")
  send_at("AT+CIPQSEND=1");
  if (wait_response() != 1)
  {
    return false;
  }

  // Set to get data manually
  send_at("AT+CIPRXGET=1");
  if (wait_response() != 1)
  {
    return false;
  }

  // Start Task and Set APN, USER NAME, PASSWORD
  send_at("AT+CSTT=\"" + apn + "AT\",\"" + user + "AT\",\"" + pwd + "AT\"");
  if (wait_response(60000L) != 1)
  {
    return false;
  }

  // Bring Up Wireless Connection with GPRS or CSD
  send_at("AT+CIICR");
  if (wait_response(60000L) != 1)
  {
    return false;
  }

  // Get Local IP Address, only assigned after connection
  send_at("AT+CIFSR;E0");
  if (wait_response(10000L) != 1)
  {
    return false;
  }

  // Configure Domain Name Server (DNS)
  send_at("AT+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\"");
  if (wait_response() != 1)
  {
    return false;
  }

  return true;
}

bool gsm_check_gprs()
{
  if (!gsm_check_network())
  {
    return false;
  }
  
  send_at("AT+SAPBR=2,1");
  String data;
  if (wait_response(10, data) == 1)
  {
    int status_index = data.indexOf("0.0.0.0");
    log_print("gprs status: " + String(status_index));
    //-1 connected.
    //else not connected. 
    if (status_index == -1)
    {
      return true;
    }
    else
    {
      return gsm_gprs_connect(apn_name);
    }
  }
  return false;
}

//-----------------------------------------------------MQTT
#include <PubSubClient.h>
String broker_ip = "109.122.233.74"; //"akranaaa.ddns.net";
uint16_t broker_port = 1883;
String broker_client_id = "esp8266_1";
String broker_username = "admin";
String broker_password = "admin";
PubSubClient mqtt_client(connection_client);

bool check_connection_mqtt_broker()
{
  if (!gsm_check_gprs())
  {
    return false;
  }
  if (!mqtt_client.connected())
  {
    log_print("mqtt not connected");
  }
  unsigned long start_time = millis();
  while (!mqtt_client.connected() && (millis() - start_time) <= 10000)
  {
    if (mqtt_client.connect(broker_client_id.c_str(), broker_username.c_str(), broker_password.c_str()))
    {
      log_print("MQTT broker connected");
    }
    else
    {
      log_print("failed with state " + String(mqtt_client.state()));
      return false;
    }
  }
  if (mqtt_client.connected())
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool mqtt_init()
{
  mqtt_client.setServer(broker_ip.c_str(), broker_port);
  return check_connection_mqtt_broker();
}

bool publish_to_mqtt_broker(const char *topic, const char *payload)
{
  if (check_connection_mqtt_broker())
  {
    return mqtt_client.publish(topic, payload);
  }
  else
  {
    return false;
  }
}

//-----------------------------------------------------GPS
#if defined(HAS_SENSOR_GPS)
#include "TinyGPS++.h"
#include <ArduinoJson.h>
#define RXGPS 3U // 3
#define TXGPS 1U // 1
SoftwareSerial gps_serial_at(RXGPS, TXGPS);
TinyGPSPlus gps_client;
String gps_topic = "gps";
unsigned long gps_check_time_publish;
unsigned long gps_interval_time_publish = 5000;

bool gps_init()
{
  gps_serial_at.begin(9600);
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

void setup()
{
  Serial.begin(115200);
  log_print("serial started");
  delay(1000);
  (gsm_init() == true) ? log_print("success : gsm initialized.") : log_print("error : gsm init failed.");
  (gsm_check_network() == true) ? log_print("success : gsm network connected.") : log_print("error : gsm network has failed to connect.");
  (gsm_check_gprs() == true) ? log_print("success : gprs connected.") : log_print("error : gprs has failed to connect.");
  (mqtt_init() == true) ? log_print("success : mqtt initialized.") : log_print("error : mqtt init failed.");
  #if defined(HAS_SENSOR_GPS)
  //(gps_init() == true) ? log_print("success : gps initialized.") : log_print("error : gps init failed.");
  #endif
}

/*
unsigned long gps_check_time_publish = millis();
unsigned long gps_interval_time_publish = 5000;
String gps_topic = "gps";
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
*/

void loop()
{
  mqtt_client.loop();
  if (millis() - gps_check_time_publish > gps_interval_time_publish)
  {
    String gps_data = String(millis()); // get_gps_data();
    //if (gps_data.length() > 50)
    //{
      publish_gps_data(gps_data);
      log_print(gps_data);
    //}
    gps_check_time_publish = millis();
  }
}
