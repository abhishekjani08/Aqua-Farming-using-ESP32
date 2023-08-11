// Necessary Libraries
#include "painlessMesh.h"
#include <ArduinoJson.h>
//#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

 
// WiFi Credentials
#define   MESH_PREFIX     "gatewayNode"
#define   MESH_PASSWORD   "somethingSneaky"
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

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;


// Needed for painless library
void receivedCallback( uint32_t from, String &msg )
{
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
  board_status = doc["board"];
  led = doc["pin"];
  led_status = doc["status"];
  Serial.print("Data "); Serial.println(led_status);
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
  DynamicJsonDocument doc(1024);
  // Tempature 
  sensors.requestTemperatures(); 
  
  Serial.print("Celsius temperature: ");
  Serial.print(sensors.getTempCByIndex(0)); 
 
  //json doc
  doc["type"] = "Data";
 
  int temp = sensors.getTempCByIndex(0);
  doc["Temp"] = temp;
  String msg ;
  serializeJson(doc, msg);
  mesh.sendBroadcast( msg );
  Serial.print("Mesh Broadcast Sensor Data - "); Serial.println(msg);
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
  Serial.begin(115200);

  pinMode(21, OUTPUT);
  pinMode(22, OUTPUT);

  sensors.begin();
  
  digitalWrite(21,LOW);
  digitalWrite(22,HIGH);


  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
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
  Serial.print("I am in Child Node");
}
