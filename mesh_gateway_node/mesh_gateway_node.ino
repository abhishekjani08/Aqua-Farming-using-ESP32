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
#define MESH_PREFIX "meshnetwork"
#define MESH_PASSWORD "123456789"
#define MESH_PORT 5555

// Gateway Node ID:

// Variables
//int led;
bool led_status;
int board_number_1;
int board_number_2;

//int board;
int pin_number;
//int pin_status;
bool message_ready = true;
String message = "";
String msg1 = "";
int temp;
double child1_temperature;
double child2_temperature;
double child3_temperature;
double child1_ph;
double child2_ph;
double child3_ph;
int pH1;
int pH2;
int pH3;
String nodeName = "";


String DO, pH, Temp, Tds;


Scheduler userScheduler; // to control your personal task
painlessMesh mesh;


// User stub
void sendMessage(); // Prototype so PlatformIO doesn't complain/ Used to Broadcast Message to all Child Nodes
void send_request(); // Sends data serially to Blynk Node

Task taskSendMessage(TASK_SECOND *1, TASK_FOREVER, &sendMessage);
Task taskSendRequest(TASK_SECOND *1, TASK_FOREVER, &send_request);

void sendMessage()
{
  uint32_t nodeId = mesh.getNodeId();
  msg1 = "Hello from Gateway Node with Node ID: " + String(nodeId);
  DynamicJsonDocument doc(1024);
  doc["board_1"] = board_number_1;
  doc["board_2"] = board_number_2;
  doc["pin"] = pin_number;
  doc["status"] = led_status;
  doc["child1_temperature"] = child1_temperature;
  doc["child2_temperature"] = child2_temperature;
  doc["child3_temperature"] = child3_temperature;
  //Serial.println("PH 1:" + pH1)
  doc["child1_ph"] = child1_ph;
  doc["child2_ph"] = child2_ph;
  doc["child3_ph"] = child3_ph;
  doc["msg1"] = msg1;

  String msg;
  serializeJson(doc, msg);
  mesh.sendBroadcast(msg);
}

void send_request()
{
  DynamicJsonDocument doc_request(1024);
  doc_request["type"] = "Data";
  String tempChild1 = String(child1_temperature, 2);
  String tempChild2 = String(child2_temperature, 2);
  String tempChild3 = String(child3_temperature, 2);
  String phChild1 = String(child1_ph, 2);
  String phChild2 = String(child2_ph, 2);
  String phChild3 = String(child3_ph, 2);
  doc_request["child1_temperature"] = tempChild1;
  doc_request["child2_temperature"] = tempChild2;
  doc_request["child3_temperature"] = tempChild3;
  doc_request["child1_ph"] = phChild1;
  doc_request["child2_ph"] = phChild2;
  doc_request["child3_ph"] = phChild3;

  //Serial.println("");
  Serial.println("\n");
  Serial.print("Sending Request - ");
  //Serial.println("IS Serial 2 available: " + Serial2.available());
  serializeJson(doc_request, Serial); //{"type":"Data","child1_temperature":0,"child2_temperature":0}
  Serial.println("");
  serializeJson(doc_request, Serial2);
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg)
{
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

  if (nodeName == "child1")
  {
    Serial.println("Child 1 if statement");
    child1_temperature = doc["child1_temperature"].as<double>();
    child1_ph = doc["child1_ph"].as<double>();
  }
  if (nodeName == "child2")
  {
    Serial.println("Child 2 if statement");
    child2_temperature = doc["child2_temperature"].as<double>();
    child2_ph = doc["child2_ph"].as<double>();
  }
  if (nodeName == "child3")
  {
    Serial.println("Child 3 if statement");
    child3_temperature = doc["child3_temperature"].as<double>();
    child3_ph = doc["child3_ph"].as<double>();
  }

  //Serial.println("");

  Serial.print("Child 1 Temp: ");
  Serial.print(child1_temperature);
  Serial.print(", Child 1 PH: ");
  Serial.println(child1_ph);

  Serial.print("Child 2 Temp: ");
  Serial.print(child2_temperature);
  Serial.print(", Child 2 PH: ");
  Serial.println(child2_ph);

  Serial.print("Child 3 Temp: ");
  Serial.print(child3_temperature);
  Serial.print(", Child 3 PH: ");
  Serial.println(child3_ph);
  
  Serial.println("Received in Gateway: " + msg1);
  serializeJson(doc, Serial); //{"type":"Data"}
  serializeJson(doc, Serial2);
}

void newConnectionCallback(uint32_t nodeId)
{
  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback()
{
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset)
{
  //Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup()
{
  Serial.begin(115200); // For Debugging purpose
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2); // For sending data to another ESP32

  //mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION );
  //mesh.setDebugMsgTypes( ERROR | STARTUP | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES );
  mesh.setDebugMsgTypes(ERROR | STARTUP);

  // Initialize the mesh network
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);

  // Get and print the Node ID
  uint32_t nodeId = mesh.getNodeId();
  Serial.printf("Node ID: %u\n", nodeId);

  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  userScheduler.addTask(taskSendRequest);
  taskSendMessage.enable();
  taskSendRequest.enable();
  // timer.setInterval(1000L, send_request);
}

void loop()
{

  if (Serial2.available())
  {
    Serial.println("Serial 2 available in Gateway Node");
    message = Serial2.readString();
    message_ready = true;
  }
  //Serial.println("");
  if (message_ready)
  {
    Serial.print("Received from Serial2: " + message);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, message);
    board_number_1 = doc["board_1"];
    board_number_2 = doc["board_2"];
    //led = doc["led"];
    led_status = doc["status"];
    pin_number = doc["pin"];

    // reason behind getting last temperature even when child node 1 is down
    child1_temperature = doc["child1_temperature"].as<double>();
    child2_temperature = doc["child2_temperature"].as<double>();
    child3_temperature = doc["child3_temperature"].as<double>();

    child1_ph = doc["child1_ph"].as<double>();
    child2_ph = doc["child2_ph"].as<double>();
    child3_ph = doc["child3_ph"].as<double>();
    String msg1 = doc["msg1"].as<String>();

    message_ready = false;
  }
  mesh.update();
  delay(10);
}
