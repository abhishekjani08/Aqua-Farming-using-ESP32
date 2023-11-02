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

// Gateway Node ID: 

// Variables
int led;
int led_status;
int board_number;
int board;
int pin;
int pin_status;
bool message_ready = true;
String message = "";
String msg1 = "";
int temp;
double child1_temperature;
double child2_temperature;
String nodeName = "";


String DO,pH,Temp,Tds;


Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;


// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain/ Used to Broadcast Message to all Child Nodes
void send_request() ; // Sends data serially to Blynk Node

Task taskSendMessage( TASK_SECOND * 1 , TASK_FOREVER, &sendMessage );
Task taskSendRequest( TASK_SECOND * 1 , TASK_FOREVER, &send_request );

void sendMessage()
{
  uint32_t nodeId = mesh.getNodeId();
  msg1 = "Hello from Gateway Node with Node ID: " + String(nodeId);
  DynamicJsonDocument doc(1024);
  doc["board"] = board_number;
  doc["pin"] = led;
  doc["status"] =  led_status;
  doc["child1_temperature"] = child1_temperature;
  doc["child2_temperature"] = child2_temperature;
  doc["msg1"] = msg1;
 
  String msg ;
  serializeJson(doc, msg);
  mesh.sendBroadcast( msg );
 
}
 
void send_request()
{
  DynamicJsonDocument doc_request(1024);
  doc_request["type"] = "Data";  
  String tempChild1 = String(child1_temperature);
  String tempChild2 = String(child2_temperature);
  doc_request["child1_temperature"] = tempChild1; 
  doc_request["child2_temperature"] = tempChild2; 
 
  Serial.print("Sending Request - ");
  //Serial.println("IS Serial 2 available: " + Serial2.available());
  serializeJson(doc_request, Serial); //{"type":"Data","child1_temperature":0,"child2_temperature":0}
  Serial.println("");
  serializeJson(doc_request, Serial2);

}


// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  //Serial.println("Received Callback of Gateway");

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

  msg1 = doc["msg1"].as<String>();
  nodeName = doc["Node Name"].as<String>();

  if(nodeName == "child1"){
    child1_temperature = doc["child1_temperature"].as<double>();
  }
  else if(nodeName == "child2"){
    child2_temperature = doc["child2_temperature"].as<double>();
  }

  Serial.print("\nChild 1 Temp: ");
  Serial.println(child1_temperature);
  Serial.print("Child 2 Temp: ");
  Serial.println(child2_temperature);
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
  //Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
  Serial.begin(115200);  // For Debugging purpose
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // For sending data to another ESP32
  
  //mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION );
  //mesh.setDebugMsgTypes( ERROR | STARTUP | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES );  
  mesh.setDebugMsgTypes(ERROR | STARTUP );
  
  // Initialize the mesh network
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );

  // Get and print the Node ID
  uint32_t nodeId = mesh.getNodeId();
  Serial.printf("Node ID: %u\n", nodeId);

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

  if(Serial2.available())
  {
    Serial.println("Serial 2 available in Gateway Node");
    message = Serial2.readString();
    message_ready = true;
  }
  //Serial.println("");
  if(message_ready){
    Serial.println("Received from Serial2: " + message); 


    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);
    board_number = doc["board_number"];
    led = doc["led"];
    led_status = doc["status"];

    // reason behind getting last temperature even when child node 1 is down
    child1_temperature = doc["child1_temperature"].as<double>();
    child2_temperature = doc["child2_temperature"].as<double>();
    String msg1 = doc["msg1"].as<String>();

    message_ready  = false;
  }
  mesh.update();
  //delay(1000);
}

