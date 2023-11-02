#define BLYNK_PRINT Serial

// Anish's Credentials
#define BLYNK_TEMPLATE_ID "TMPL3y-A5yv"
#define BLYNK_TEMPLATE_NAME "Aqua Farming"
#define BLYNK_AUTH_TOKEN "WfQITWPhO1JeF3zrRGXvt09vi14Ek"

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
//double child1_temperature;
//double child2_temperature;

#define LED_PIN 2  // LED is usually connected to D2 pin. Change if needed.

BLYNK_WRITE(V0) {
  board = 0;
  pin = LED_PIN;
  pin_status = param.asInt();  // Pin Status 1/0
  Serial.println("\nV0 On");
}


// Data Coming from Blynk App
BLYNK_WRITE(V1) {
  board = 1;
  pin = LED_PIN;
  pin_status = param.asInt();  // Pin Status 1/0
  //Serial.println(pin_status);
  Serial.println("\nV1 On");
}

BLYNK_WRITE(V2) {
  board = 2;
  pin = LED_PIN;
  pin_status = param.asInt();
  Serial.println("\nV2 On");
}


void setup() {
  // Debug console
  Serial.begin(115200);                           // For Debugging purpose
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);  // For sending data to another ESP32

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);  // Establishing Communication with Blynk Server
}

void loop() {

  if(Serial2.available())  //
  {
    message = Serial2.readString();
    //Serial.println(message);
    //Serial.println("");
    Serial.println("\nSerial2 available in blynk node:");
    Serial.println("\n" + message);
    messageReady = true;
    //break;
  }

  // Only process message if there's one
  if (messageReady) {
    
    // The only messages we'll parse will be formatted in JSON
    DynamicJsonDocument doc(1024);  // ArduinoJson version 6+
    // Attempt to deserialize the message
    DeserializationError error = deserializeJson(doc, message);

    Serial.println("Received from Serial2: " + message);

    if (!error) {
      if (doc["type"] == "Data") {

        Serial.println("Received Type = Data in Blynk Node");

        double child1_temperature = doc["child1_temperature"].as<double>();
        double child2_temperature = doc["child2_temperature"].as<double>();

        //doc.clear(); // Clear the document for the response

        doc["type"] = "response";
        // Get data from virtual pin
        doc["board_number"] = board;
        doc["led"] = pin;
        doc["status"] = pin_status;
        doc["child1_temperature"] = child1_temperature;
        doc["child2_temperature"] = child2_temperature;


        Serial.println("Temperature Child 1: " + String(child1_temperature));
        Serial.println("Temperature Child 2: " + String(child2_temperature));


        serializeJson(doc, Serial2);  // Sending data to another ESP32
        //Serial.println("");
        Serial.println("Sending Data - ");
        serializeJson(doc, Serial);  //{"type":"response","board_number":1/2,"led": pin_number, "status": 1/0}

        // Update Blynk virtual pin
        Blynk.virtualWrite(V5, child1_temperature);
        Blynk.virtualWrite(V6, child2_temperature);
        // Get data from virtual pin
        // Blynk.virtualWrite(V3,doc["DO"].as<String>());
        // Blynk.virtualWrite(V4,doc["pH"].as<String>());
        // //Blynk.virtualWrite(V5,doc["Temp"].as<String>());
        // Blynk.virtualWrite(V6,doc["Tds"].as<String>());
      }
      messageReady = false;
    } else {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      messageReady = false;
      return;
    }
  }

  Blynk.run();  // Handling Blynk Services
}