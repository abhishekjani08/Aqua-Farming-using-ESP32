#define BLYNK_PRINT Serial

// Anish's Credentials
#define BLYNK_TEMPLATE_ID "TMPL3y-A5yv23"
#define BLYNK_TEMPLATE_NAME "Aqua Farming"
#define BLYNK_AUTH_TOKEN "WfQITWPhO1JeF3zrRGXvt09vi14Ekms-"

// Necessary Libraries
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ArduinoJson.h>
#include <TaskScheduler.h> // Include the TaskScheduler library


// Serial2 pins of ESP32
#define RXD2 16
#define TXD2 17

Scheduler userScheduler;

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
double child2_temperature;
double child2_ph;
double child3_temperature;
double child3_ph;

#define LED_PIN 2  // LED is usually connected to D2 pin. Change if needed.

BLYNK_WRITE(V0) {
  board = 0;
  pin = LED_PIN;
  pin_status = param.asInt();  // Pin Status 1/0
  Serial.println("V0 On");
}


// Data Coming from Blynk App
// BLYNK_WRITE(V3) function to handle button press for motor 1
BLYNK_WRITE(V3) {
  board = 1;
  pin = LED_PIN;
  pin_status = param.asInt();  // Pin Status 1/0

  if (pin_status == 1)
  { // If button is turned on
    Serial.println("Motor 1 ON");
  }
  else
  { // If button is turned off
    Serial.println("Motor 1 OFF");
  }
}

// BLYNK_WRITE(V6) function to handle button press for motor 2
BLYNK_WRITE(V6) {
  board = 2;
  pin = LED_PIN;
  pin_status = param.asInt();  // Pin Status 1/0

  if (pin_status == 1)
  { // If button is turned on
    Serial.println("Motor 2 ON");
  }
  else
  { // If button is turned off
    Serial.println("Motor 2 OFF");
  }
}

// BLYNK_WRITE(V9) function to handle button press for motor 3
BLYNK_WRITE(V9) {
  board = 3;
  pin = LED_PIN;
  pin_status = param.asInt();  // Pin Status 1/0
  
  if (pin_status == 1)
  { // If button is turned on
    Serial.println("Motor 3 ON");
  }
  else
  { // If button is turned off
    Serial.println("Motor 3 OFF");
  }
}

Task taskRunBlink(TASK_SECOND * 1, TASK_FOREVER, &runBlink);

void setup()
{
  // Debug console
  Serial.begin(115200); // For Debugging purpose
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // For sending data to another ESP32
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);    // Establishing Communication with Blynk Server
  userScheduler.addTask( taskRunBlink );
  taskRunBlink.enable();
}

void runBlink() {
  Blynk.run(); // Handling Blynk Services
  Serial.println("No Serial Communication");
}

void loop()
{
  while (Serial2.available()) //
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
      Serial.println("Received Data from Gateway in Blynk Node");

      double temperature1 = doc["child1_temperature"].as<double>();
      double temperature2 = doc["child2_temperature"].as<double>();
      double temperature3 = doc["child3_temperature"].as<double>();
      double ph1 = doc["child1_ph"].as<double>();
      double ph2 = doc["child2_ph"].as<double>();
      double ph3 = doc["child3_ph"].as<double>();

      //Serial.println("");

      Serial.print("Child 1: Temp = ");
      Serial.print(temperature1);
      Serial.print(" | PH =  ");
      Serial.println(ph1);
      
      Serial.print("Child 2: Temp = ");
      Serial.print(temperature2);
      Serial.print(" | PH =  ");
      Serial.println(ph2);

      Serial.print("Child 3: Temp = ");
      Serial.print(temperature3);
      Serial.print(" | PH =  ");
      Serial.println(ph3);

      //Serial.println("Temperature Child 1- " + String(temperature1));
      //Serial.println("Ph Child 1- " + String(ph1));
      //Serial.println("Temperature Child 2- " + String(temperature2));
      //Serial.println("Ph Child 2- " + String(ph2));
      //Serial.println("Temperature Child 3- " + String(temperature3));
      //Serial.println("Ph Child 3- " + String(ph3));

      doc["type"] = "response";
      // Get data from virtual pin
      doc["board"] = board;
      doc["pin"] = pin;
      doc["status"] = pin_status;
      doc["child1_ph"] = ph1;
      doc["child1_temperature"] = temperature1;
      doc["child2_ph"] = ph2;
      doc["child2_temperature"] = temperature2;
      doc["child3_ph"] = ph3;
      doc["child3_temperature"] = temperature3;

      // Sending data to another ESP32
      String json;
      serializeJson(doc, json);
      Serial.println("Sending Data: " + json);
      Serial2.println(json);

      // Update Blynk virtual pin
      Blynk.virtualWrite(V1, temperature1);
      Blynk.virtualWrite(V4, temperature2);
      Blynk.virtualWrite(V7, temperature3);
      Blynk.virtualWrite(V2, ph1);
      Blynk.virtualWrite(V5, ph2);
      Blynk.virtualWrite(V8, ph3);

    }
    messageReady = false;
  }
  //delay(1000);
  
}
