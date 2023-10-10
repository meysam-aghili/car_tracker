#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
//#include <StreamDebugger.h>
#include <PubSubClient.h>
#include <TinyGPSPlus.h>
#include <ArduinoJson.h>

#define GPS_POWER_PIN 16
#define SerialMon Serial
#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_DEBUG SerialMon
#include <TinyGsmClient.h>
SoftwareSerial gsm_serial(14, 12);  // RX, TX D5, D6
//StreamDebugger debugger(gsm_serial, SerialMon);

//TinyGsm        modem(debugger);
TinyGsm        modem(gsm_serial);
TinyGsmClient modem_client(modem);
PubSubClient  modem_mqtt(modem_client);

WiFiClient wifi_client;
PubSubClient  wifi_mqtt(wifi_client);

SoftwareSerial gps_serial(4, 5); // D2 , D1
//StreamDebugger gps_debugger(gps_serial, SerialMon);
TinyGPSPlus gps;

unsigned long gps_check_time = millis();


// Add a reception delay, if needed.
// This may be needed for a fast processor at a slow baud rate.
//#define TINY_GSM_YIELD() { delay(2); }

String default_communication = "wifi"; // gprs wifi

String apn = "mtnirancell";
String gprs_user = "";
String gprs_pass = "";
String gsm_pin_code = "21831454";

String wifi_ssid = "tplink_";
String wifi_pass = "Sh!@#123";

String broker = "192.168.1.102";//"akrana.ddns.net"; //if local wifi we must use 192.168.1.<>
uint16_t broker_port = 1883;
String broker_client_id = "GsmClientTest";
String broker_user = "admin";
String broker_pass = "admin";
String topicTest = "test";

auto add_zero = [](String a) {
        if (a.length()==1)
        {
          return "0"+a;
        }
          else{return a;}
      };
String gps_json_data;

boolean check_connection_gsm(){
  if (!modem.isNetworkConnected())
  {
    SerialMon.println("no network");
    SerialMon.println("connecting to modem network...");
  
    if (!modem.waitForNetwork(180000L)) 
    {
      SerialMon.println("modem network failed");
      delay(5000);
      return 0;
    }
    else
    { 
      SerialMon.println("modem network connected"); 

      return 1;
    }
  }
  else{return 1;}
}

boolean check_connection_gprs(){
  if (!modem.isGprsConnected())
  {
    SerialMon.println("no gprs network");
    SerialMon.println("connecting to ");
    SerialMon.println(apn);
    boolean status = modem.gprsConnect(apn.c_str(), gprs_user.c_str(), gprs_pass.c_str());
    if (!status) {
      SerialMon.println("gprs connection attempt failed");
      delay(10000);
      return 0;
    }
    else 
    { 
      SerialMon.println("gprs connected"); 
      return 1;
    }
  }
  else{return 1;}
}

boolean check_connection_gsm_gprs(){
  if (check_connection_gsm())
  {
    return check_connection_gprs();
  }
  else
  {
    return 0;
  }
}

boolean check_connection_wifi(){
  if(WiFi.status() != WL_CONNECTED)
  {
    SerialMon.println("no wifi network");
    SerialMon.print("connecting to ");
    SerialMon.print(wifi_ssid);
    WiFi.begin(wifi_ssid, wifi_pass);
    unsigned long start_time = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start_time) <= 50000)
    {
      delay(500);
      SerialMon.print(".");
    }
    if (WiFi.status() != WL_CONNECTED) 
    {
      SerialMon.println("");
      SerialMon.println("wifi connection attempt failed");
      delay(5000);
      return 0;
    }
    else
    {
      SerialMon.println("");
      SerialMon.println("wifi connected");
      SerialMon.print("ip address is: ");
      SerialMon.println(WiFi.localIP());
      delay(100);
      return 1;
    }
  }
  else{return 1;}
}

boolean check_connection(){
  if (!strcmp(default_communication.c_str(),"gprs")) 
  {
    return check_connection_gsm_gprs();
  }
  else if (!strcmp(default_communication.c_str(),"wifi"))
  {
    return check_connection_wifi();
  }
  else
  {
    SerialMon.println("error : choose correct default_communication");
    return 0;
  }
}

boolean check_connection_mqtt_broker(PubSubClient& mqtt) 
{
  if (!mqtt.connected()) 
  {
    SerialMon.println("mqtt broker not connected.");
    SerialMon.print("connecting to ");
    SerialMon.println(broker);
    unsigned long start_time = millis();
    while (!mqtt.connected() && (millis() - start_time) <= 10000)
    {
      mqtt.connect(broker_client_id.c_str(), broker_user.c_str(), broker_pass.c_str());
      delay(500);
    }
    if (!mqtt.connected())
    {
      SerialMon.println("mqtt broker connection attempt failed");
      delay(5000);
      return 0;
    }
    else
    {
      SerialMon.println("mqtt broker connected");
      delay(100);
      return 1;
    }
  }
  else{return 1;}
}

boolean setup_modem()
{
  SerialMon.println("initializing modem");
  modem.init();
  //SerialMon.print("modem info: ");
  //SerialMon.println(modem.getModemInfo());
  //SerialMon.print("sim status: ");
  //SerialMon.println(modem.getSimStatus());
  if (modem.getSimStatus() == 3)
  {
    modem.simUnlock(gsm_pin_code.c_str()); 
  }
  return check_connection_gsm_gprs();
}

boolean setup_wifi()
{
  WiFi.disconnect();
  SerialMon.println("initializing wifi");
  return check_connection_wifi();
}

boolean setup_connection()
{
  if (!strcmp(default_communication.c_str(),"gprs")) {
    return setup_modem();
  }
  else if (!strcmp(default_communication.c_str(),"wifi"))
  {
    return setup_wifi();
  }
  else
  {
    SerialMon.println("error : choose correct default_communication");
    return 0;
  }
}

void mqtt_callback(char* topic, byte* payload, unsigned int len) {
  SerialMon.print("message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();
}

boolean setup_mqtt_broker(){
  if (!strcmp(default_communication.c_str(),"gprs"))
  {
    SerialMon.println("initializing mqtt broker");
    modem_mqtt.setServer(broker.c_str(), broker_port);
    //modem_mqtt.setCallback(mqtt_callback);
    return check_connection_mqtt_broker(modem_mqtt);
  }
  else if (!strcmp(default_communication.c_str(),"wifi"))
  {
    SerialMon.println("initializing mqtt broker");
    wifi_mqtt.setServer(broker.c_str(), broker_port);
    //wifi_mqtt.setCallback(mqtt_callback);
    return check_connection_mqtt_broker(wifi_mqtt);
  }
  else
  {
    SerialMon.println("error : choose correct default_communication");
    return 0;
  }
}

boolean get_gps_data()
{
  while (gps_serial.available()>0) 
  {
    if (gps.encode(gps_serial.read()) && gps.location.isValid() && gps.time.isValid()) 
    {
      //DynamicJsonDocument doc(1024);
      //doc["datetime"] = String(gps.date.year())+"-"+add_zero(String(gps.date.month()))+"-"+add_zero(String(gps.date.day()))+" "+add_zero(String(gps.time.hour()))+":"+add_zero(String(gps.time.minute()))+":"+add_zero(String(gps.time.second()));
      //doc["latitude"] = gps.location.lat();
      //doc["longitude"] = gps.location.lng();
      //doc["satellites"] = gps.satellites.value();
      //doc["altitude"] = gps.altitude.meters();
      //doc["speed"] = gps.speed.knots();
      //doc["course"] = gps.course.deg();
      //doc["hdop"] = gps.hdop.value();
      //serializeJson(doc, gps_json_data);
      //SerialMon.println(gps_json_data);
      SerialMon.println("got gps");
      gps_check_time = millis();
      return true;
    }
    else
    {
      if ((millis()-gps_check_time) >=10000)
      {
        SerialMon.print(gps.encode(gps_serial.read()));
        SerialMon.print(gps.location.isValid());
        SerialMon.println(gps.time.isValid());
        SerialMon.println(gps.satellites.value());
        SerialMon.println(millis());
        SerialMon.println("no gps data");
        gps_check_time = millis();
      }
      return false;
    }
  }
  return false;
}

boolean mqtt_publish(const char *topic, const char *payload)
{
  if (!strcmp(default_communication.c_str(),"gprs"))
  {
    if (check_connection_mqtt_broker(modem_mqtt))
    {
      return modem_mqtt.publish(topic, payload);
    }
    else{return 0;}
  }
  else if (!strcmp(default_communication.c_str(),"wifi"))
  {
    //if (check_connection_mqtt_broker(wifi_mqtt))
    //{
      //wifi_mqtt.publish(topic, payload);
      delay(1000);
      SerialMon.println("before pub");
      //if (!wifi_mqtt.connected()) 
      //{
       // SerialMon.println("before pub2");
       // wifi_mqtt.connect("testclient", "admin", "admin");
      //}
      //SerialMon.println("before pub3");
      return wifi_mqtt.publish("test2", "ok");
    //}
    //else{return 0;}
  }
  else
  {
    SerialMon.println("error : choose correct default_communication");
    return 0;
  }
}

boolean send_gps_data()
{
  //gps.location.isUpdated()
  //if (get_gps_data())
  //{
    SerialMon.println("sending data");
    return mqtt_publish(topicTest.c_str(), "{}"); //gps_json_data.c_str()
  //}
  //return 0;
}

void setup() {
  //pinMode(GPS_POWER_PIN ,OUTPUT);
  //digitalWrite(GPS_POWER_PIN ,LOW);
  ESP.wdtDisable(); //disable software wdt to prevent resets
  ESP.wdtEnable(8000); //enable software wdt and set time to 8 second to prevent resets
  SerialMon.begin(115200);
  SerialMon.println("Wait...");
  
  gsm_serial.begin(115200);
  delay(1000);
  if (!setup_connection())
  {
    SerialMon.println("setup connection failed");
    ESP.reset();
  }
  SerialMon.println("setup connection succeded");

  //digitalWrite(GPS_POWER_PIN ,HIGH);
  gps_serial.begin(9600);
  delay(1000);
  
  if (!setup_mqtt_broker())
  {
    SerialMon.println("setup mqtt broker failed");
    ESP.reset();
  }
  SerialMon.println("setup mqtt broker succeded");
  
  SerialMon.println("setup succeded");
}

void loop() {
  
  ESP.wdtFeed();
  //if (!check_connection())
  //{
  //  SerialMon.println("connection failed");
  //}
  
  if (!strcmp(default_communication.c_str(),"gprs"))
  {
    modem_mqtt.loop();
  }
  else if (!strcmp(default_communication.c_str(),"wifi"))
  {
    wifi_mqtt.loop();
  }
  else
  {
    SerialMon.println("error : choose correct default_communication");
    return;
  }
  
  
  SerialMon.println(send_gps_data());

  //get_gps_data();
  
  
}

