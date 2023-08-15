// Necessary Libraries
#include "painlessMesh.h"
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiClient.h>


// Serial2 pins of ESP32
#define RXD2 16
#define TXD2 17

// WiFi Credentials for Mesh Networking
#define   MESH_PREFIX     "meshnetwork"
#define   MESH_PASSWORD   "123456789"
#define   MESH_PORT       5555


// Variables
int led;
int led_status;
int board_status;
int board;
int pin;
int pin_status;
bool message_ready = true;
bool message_received = false;
String message = "";
String msg1 = "";
int temp;

String DO,pH,Temp,Tds;


Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;


// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
void send_request() ;

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskSendRequest( TASK_SECOND * 1 , TASK_FOREVER, &send_request );

void sendMessage()
{
  msg1 = "Hello from Gateway Node";
  DynamicJsonDocument doc(1024);
  doc["board"] = board_status;
  doc["pin"] = led;
  doc["status"] =  led_status;
  doc["Temp"] = temp;
  doc["msg1"] = msg1;
  //doc["status_1"] = led_status_1;
  String msg ;
  serializeJson(doc, msg);
  mesh.sendBroadcast( msg );
  //Serial.println("Gateway to Mesh Broadcast - " + msg);

 // taskSendMessage.setInterval((TASK_SECOND * 1));
}


// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.println("Received Callback of Gateway");

  //Deserializing
  String json;
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }

  // Serial.println("Before Printing JSON");
  // Serial.println(json);

  // // Print the JSON data to Serial
  // Serial.println("Received JSON:");
  // serializeJson(doc, Serial);
  // Serial.println();

  // // Clear the contents of Serial2 and print the JSON data
  // Serial2.print("Received JSON: ");
  // serializeJson(doc, Serial2);
  // Serial2.println();
  
  // Serial.println("Before Printing JSON");
  // Serial.println(json);
  
  // if (doc["type"] == "Data")
  // //if (doc["type"].as<String>().equalsIgnoreCase("Data"))
  // {
  //   int temp = doc["Temp"];
  //   // Forward temperature data to Blynk node
  //   //Blynk.virtualWrite(V5, temp);  // Assuming V5 is your Blynk virtual pin for temperature
  //   Serial.println("Got Temperature as " + temp);
  // }
  //int temp;
  //String msg1 = "";
  // doc["type"] = "Data"; 
  // doc["Temp"] = temp;
  // doc["msg1"] = msg1;
  msg1 = doc["msg1"].as<String>();
  temp = doc["Temp"];
  Serial.println("Received in Gateway: " + msg1);
  serializeJson(doc, Serial); //{"type":"Data"}
  serializeJson(doc, Serial2);

  }

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
  Serial.begin(115200);  // For Debugging purpose
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // For sending data to another ESP32
  
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION );
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  userScheduler.addTask( taskSendRequest );
  taskSendMessage.enable();
  taskSendRequest.enable();
  // timer.setInterval(1000L, send_request);
}

void loop()
{

  if (Serial2.available())
  {
    message = Serial2.readString();
    message_ready = true;
  }
  //Serial.println("");
  //Serial.println("Received from Serial2 - " + message); 

  //delay(5000); // delay here

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, message);
  board_status = doc["board_status"];
  led = doc["led"];
  led_status = doc["status"];
  //temp = doc["Temp"]; 
  //msg1 = doc["msg1"];
  int temp = doc["Temp"].as<int>(); // Assuming Temp is of type int
  String msg1 = doc["msg1"].as<String>();



  // it will run the user scheduler as well
  mesh.update();
  //timer.run();
}

void send_request()
{
  DynamicJsonDocument doc_request(1024);
  doc_request["type"] = "request";  
  Serial.print("Sending Request - ");
  serializeJson(doc_request, Serial); //{"type":"request"}
  serializeJson(doc_request, Serial2);
  //taskSendMessage.setInterval((TASK_SECOND * 1));
}