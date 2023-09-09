#include <SoftwareSerial.h>
#include <StreamDebugger.h>
#include <ArduinoJson.h>

#define SerialMon Serial
#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_DEBUG SerialMon
#include <TinyGsmClient.h>
SoftwareSerial SerialAT(14, 12);  // RX, TX D5, D6
StreamDebugger debugger(SerialAT, SerialMon);

TinyGsm        modem(debugger);
//TinyGsm        modem(SerialAT);
TinyGsmClient modem_client(modem);

String gsm_pin_code = "21831454";
String apn = "mtnirancell";
String gprs_user = "";
String gprs_pass = "";

void setup() {
  SerialMon.begin(115200);
  delay(10);
  SerialMon.println("Wait...");
  SerialAT.begin(4800);
  delay(6000);
  SerialMon.println("initializing modem");
  modem.init();
  SerialMon.print("modem info: ");
  SerialMon.println(modem.getModemInfo());
  SerialMon.print("sim status: ");
  SerialMon.println(modem.getSimStatus());
  if (modem.getSimStatus() == 3)
  { 
    modem.simUnlock(gsm_pin_code.c_str()); 
  }
  if (!modem.isNetworkConnected()) 
  {
    SerialMon.println("no network");
    SerialMon.println("connecting to modem network...");
    if (!modem.waitForNetwork())
    {
      SerialMon.println("modem network failed");
      delay(5000);
    }
    else
    { 
      SerialMon.println("modem network connected");
    }
  }

  if (modem.isNetworkConnected())
  {
    SerialMon.println("no gprs network");
    SerialMon.println("connecting to ");
    SerialMon.println(apn);
    boolean status = modem.gprsConnect(apn.c_str(), gprs_user.c_str(), gprs_pass.c_str());
    if (!status) {
      SerialMon.println("gprs connection attempt failed");
      delay(10000);
    }
    else 
    { 
      SerialMon.println("gprs connected"); 
    }
  }
}

void loop() {
  /*
  if (!modem.isNetworkConnected()) 
  {
    SerialMon.println("no network");
    SerialMon.println("connecting to modem network...");
    if (!modem.waitForNetwork())
    {
      SerialMon.println("modem network failed");
      delay(5000);
    }
    else
    { 
      SerialMon.println("modem network connected");
    }
  }
  */
 SerialMon.println(modem.isNetworkConnected());
 SerialMon.println(modem.isGprsConnected());
}

