#define BLYNK_PRINT Serial

// Anish's Credentials
#define BLYNK_TEMPLATE_ID "TMPL3y-A5yv23"
#define BLYNK_TEMPLATE_NAME "Aqua Farming"
#define BLYNK_AUTH_TOKEN "WfQITWPhO1JeF3zrRGXvt09vi14Ekms-"

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

#define LED_PIN 2           // LED is usually connected to D2 pin. Change if needed.

BLYNK_WRITE(V0)
{
  //int value = param.asInt();
  
  //Serial.print("Value is ");
  //Serial.println(value);
  // Local variable `value` stores the incoming LED switch state (1 or 0)
  // Based on this value, the physical LED on the board will be on or off:

  // if (value == 1)
  // {
  //   digitalWrite(LED_PIN, HIGH);
  //   Serial.println("LED High");
  // }
  // else
  // {
  //   digitalWrite(LED_PIN, LOW);
  //   Serial.println("LED Low");
  // }

  board = 0;
  pin = LED_PIN;
  pin_status = param.asInt();   // Pin Status 1/0
  Serial.println("V0 On");
}


// Data Coming from Blynk App
BLYNK_WRITE(V1)
{
  board = 1;
  pin = LED_PIN;
  pin_status = param.asInt();   // Pin Status 1/0
  //Serial.println(pin_status);
  Serial.println("V1 On");
}

BLYNK_WRITE(V2)
{
  board = 2;
  pin = LED_PIN;
  pin_status = param.asInt();
  Serial.println("V2 On");
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
    //Serial.println(message);
    messageReady = true;
    //Serial.println("");
    Serial.println("Serial2 available in blynk node:" + message); 
    //Serial.println(message);
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
      Serial.println("Received Request from other ESP board");
      doc["type"] = "response";
      // Get data from virtual pin
      doc["board_number"] = board;
      doc["led"] = pin;
      doc["status"] = pin_status;
      
      serializeJson(doc, Serial2); // Sending data to another ESP32
      Serial.println("Sending Data - "); 
      serializeJson(doc, Serial); //{"type":"response","board_status":1/2,"led": pin_number, "status": 1/0}
      //Serial.println();
    }
    
    if (doc["type"] == "Data")
    {
        Serial.println("Received Type = Data in Blynk Node");
      
        int temperature = doc["Temp"].as<int>();
        Serial.println("Temperature: " + String(temperature));

        doc["type"] = "response";
        // Get data from virtual pin
        doc["board_number"] = board;
        doc["led"] = pin;
        doc["status"] = pin_status;

        serializeJson(doc, Serial2); // Sending data to another ESP32
        //Serial.println("");
        Serial.println("Sending Data - "); 
        serializeJson(doc, Serial); //{"type":"response","board_status":1/2,"led": pin_number, "status": 1/0}

        // Update Blynk virtual pin
        //Blynk.virtualWrite(V5, temperature);
        // Get data from virtual pin
        // Blynk.virtualWrite(V3,doc["DO"].as<String>());
        // Blynk.virtualWrite(V4,doc["pH"].as<String>());
        // //Blynk.virtualWrite(V5,doc["Temp"].as<String>());
        // Blynk.virtualWrite(V6,doc["Tds"].as<String>());
    }
    messageReady = false;
  }

  Blynk.run(); // Handling Blynk Services
}