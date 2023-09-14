// Necessary Libraries
#include "painlessMesh.h"
#include <ArduinoJson.h>
//#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

 
// WiFi Credentials
#define   MESH_PREFIX     "meshnetwork"
#define   MESH_PASSWORD   "123456789"
#define   MESH_PORT       5555

// Data wire is plugged into port 4
#define ONE_WIRE_BUS 5

// Setup a oneWire instance to communicate with any OneWire devices 
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

int led;
int led_status = 0;
int board_status = 0;
String msg1 = "";
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;


// Needed for painless library
void receivedCallback( uint32_t from, String &msg)
{
  //Serial.println("receivedCallback of Child Node");

  //Serial.printf("Received from Gatewau msg=%s \n", from, msg.c_str());
  //Deserializing
  String json;
  //String msg1 = "";
  DynamicJsonDocument doc(1024);
  json = msg.c_str();
  DeserializationError error = deserializeJson(doc, json);
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
  }
  board_status = doc["board"];
  led = doc["pin"];
  led_status = doc["status"];
  msg1 = doc["msg1"].as<String>();

  ///Serial.print("Data "); 

  Serial.println("Received in Child Node 2: " + msg1);
  //Serial.println(msg1);
  if (board_status == 1){
     digitalWrite(21, !led_status);
    if(led_status == 1){
      digitalWrite(22, HIGH);
      }
    else{
      digitalWrite(22, LOW);
      } 
  }
}
Task taskSendMessage( TASK_SECOND * 1, TASK_FOREVER, &sendMessage );

void sendMessage()
{
  msg1 = "Hello from Child Node 2";
  //mesh.sendBroadcast(msg1);
  DynamicJsonDocument doc(1024);
  // Tempature 
  //json doc
  doc["type"] = "Data";
  sensors.requestTemperatures(); 
  
  //Serial.print("Celsius temperature: ");
  //Serial.print(sensors.getTempCByIndex(0)); 
 
 
  int temp = sensors.getTempCByIndex(0);
  doc["Temp"] = temp;
  doc["msg1"] = msg1;
  String msg ;
  serializeJson(doc, msg);
  mesh.sendBroadcast( msg );
  //Serial.println("");
  //Serial.println("Mesh Broadcast Sensor Data from Child Node - " + msg);
}


void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("--> startHere: New Connection 2, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

void setup() {
  Serial.begin(115200);

  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);

  sensors.begin();
  
  digitalWrite(21,LOW);
  digitalWrite(22,HIGH);


  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
 
}

void loop() {
  // it will run the user scheduler as well
  
  mesh.update();
}