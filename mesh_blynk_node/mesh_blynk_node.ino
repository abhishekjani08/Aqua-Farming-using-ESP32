
#define BLYNK_PRINT Serial
// #define BLYNK_TEMPLATE_ID "TMPLI35ikowW"
// #define BLYNK_DEVICE_NAME "Espmesh"
// #define BLYNK_AUTH_TOKEN "dkv2bkLAqd6-euOs-cUqYCpzcuj8KjQY"

#define BLYNK_TEMPLATE_ID "TMPL3RH7UWPt0"
#define BLYNK_TEMPLATE_NAME "Temperature Sensor"
#define BLYNK_AUTH_TOKEN "cRDzju7JmD3E6Cu_lF8puumCSzTixBug"

// Necesaary Libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoJson.h>

// Serial2 pins of ESP32
#define RXD2 16
#define TXD2 17

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "iot";
char pass[] = "123456789";

// Variables
int board;
int pin;
bool pin_status;
String message = "";
bool messageReady = false;


// Data Coming fro Blynk App
BLYNK_WRITE(V1)
{
  board = 1;
  pin = 22;
  pin_status = param.asInt();
  Serial.println(pin_status);
}

BLYNK_WRITE(V2)
{
  board = 1;
  pin = 21;
  pin_status = param.asInt();
}


void setup()
{
  // Debug console
  Serial.begin(115200); // For Debugging purpose
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // For sending data to another ESP32

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Establishing Communication with Blynk Server
}
  
void loop()
{

  while (Serial2.available()) //
  {
    message = Serial2.readString();
//    Serial.println(message);
    messageReady = true;
    Serial.print("Received message - "); 
//    Serial.println(message);
  }

  // Only process message if there's one
  if (messageReady)
  {
    // The only messages we'll parse will be formatted in JSON
    DynamicJsonDocument doc(1024); // ArduinoJson version 6+
    // Attempt to deserialize the message
    DeserializationError error = deserializeJson(doc, message);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      messageReady = false;
      return;
    }

    // If request is received from another ESP32 board
    if (doc["type"] == "request")
    {
      Serial.println("Received Request");
      doc["type"] = "response";
      // Get data from virtual pin
      doc["board_status"] = board;
      doc["led"] = pin;
      doc["status"] = pin_status;
      serializeJson(doc, Serial2); // Sending data to another ESP32
      Serial.println("Sending Data - "); serializeJson(doc, Serial); //{"type":"response","board_status":1/2,"led": pin_number, "status": 1/0}
    }
     if (doc["type"] == "Data")
    {
      Serial.println("Received Data");
      // Get data from virtual pin
        Blynk.virtualWrite(V3,doc["DO"].as<String>());
        Blynk.virtualWrite(V4,doc["pH"].as<String>());
        Blynk.virtualWrite(V5,doc["Temp"].as<String>());
        Blynk.virtualWrite(V6,doc["Tds"].as<String>());
    }
    messageReady = false;
  }

  Blynk.run(); // Handeling Blynk Services
}
