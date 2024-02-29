#define BLYNK_PRINT Serial

// Blynk's Credentials Major Project
#define BLYNK_TEMPLATE_ID "TMPL3c4acbmrJ"
#define BLYNK_TEMPLATE_NAME "Major Project"
#define BLYNK_AUTH_TOKEN "sjiDRT8_iO0iq4fb3WObFFD0GzzU635t"

// Necessary Libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoJson.h>

// Serial2 pins of ESP32
#define RXD2 16
#define TXD2 17

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
char ssid[] = "iot";
char pass[] = "123456789";

// Variables
int board;
int pin;
bool pin_status;
String message = "";
bool messageReady = false;
double child1_temperature;
double child1_ph;


#define LED_PIN 2  // LED is usually connected to D2 pin. Change if needed.

// BLYNK_WRITE(V0) {
//   board = 0;
//   pin = LED_PIN;
//   pin_status = param.asInt();  // Pin Status 1/0
//   Serial.println("V0 On");
// }


// Data Coming from Blynk App
BLYNK_WRITE(V1) {
  board = 1;
  pin = LED_PIN;
  pin_status = param.asInt();  // Pin Status 1/0
  //Serial.println(pin_status);
  Serial.println("\nV1 On");
}

// BLYNK_WRITE(V2) {
//   board = 2;
//   pin = LED_PIN;
//   pin_status = param.asInt();
//   Serial.println("\nV2 On");
// }

void setup()
{
  // Debug console
  Serial.begin(115200); // For Debugging purpose
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // For sending data to another ESP32

  Serial.println("Setup started...");

  // Print message while searching for WiFi
  Serial.println("Searching for WiFi...");

  // Establishing Communication with Blynk Server
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  Serial.println("Blynk.begin executed...");

  // Print message when connected to WiFi along with the Wi-Fi name
  Serial.print("Connected to: ");
  Serial.println(WiFi.SSID());

  Serial.println("Setup completed!");
}


void loop()
{
  Serial.println("No Serial Communication");
  while (Serial2.available()) 
  {
    message = Serial2.readString();
    //Serial.println(message);
    messageReady = true;

    Serial.println("Serial2 available in blynk node:");
    // Serial.println(message);
    // Serial.println();
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
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      messageReady = false;
      return;
    }

    if (doc["type"] == "Data")
    {
      Serial.println("Received Type = Data in Blynk Node");

      double temperature1 = doc["child1_temperature"].as<double>();
      Serial.println("Temperature Child 1- " + String(temperature1));

      double ph1 = doc["child1_ph"].as<double>();
      Serial.println("Ph Child 1- " + String(ph1));

      doc["type"] = "response";
      // Get data from virtual pin
      doc["board_status"] = board;
      doc["led"] = pin;
      doc["status"] = pin_status;
      doc["child1_ph"] = ph1;
      doc["child1_temperature"] = temperature1;
     
      // Sending data to another ESP32
      String json;
      serializeJson(doc, json);
      Serial.println("Sending Data: " + json);
      Serial2.println(json);

      // Update Blynk virtual pin
      Blynk.virtualWrite(V0, temperature1);
      Blynk.virtualWrite(V1, ph1);
  
    }
    messageReady = false;
  }
  delay(1000);
  Blynk.run(); // Handling Blynk Services
}