// Necessary Libraries
#include "painlessMesh.h"
#include <ArduinoJson.h>
//#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

 
// WiFi Credentials
#define   MESH_PREFIX     "meshnetwork2"
#define   MESH_PASSWORD   "123456789"
#define   MESH_PORT       5555
#define LED_PIN 2 

// Data wire is plugged into port 5
#define ONE_WIRE_BUS 5

const int potPin = 36;
double ph;
float Value = 0.0;

// Setup a oneWire instance to communicate with any OneWire devices 
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

int led;
int led_status = 0;
int board_number = 1;
String msg1 = "";
String nodeName = "child1";
Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;
double child1_temperature;
// double child2_temperature;
double temp1;
double child1_ph;
// double child2_ph;


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
  board_number = doc["board"];
  led = doc["pin"];
  led_status = doc["status"];
  msg1 = doc["msg1"].as<String>();
  child1_temperature = doc["child1_temperature"].as<double>();
  // child2_temperature = doc["child2_temperature"].as<double>();
  child1_ph = doc["child1_ph"].as<double>();
  // child2_ph = doc["child2_ph"].as<double>();

  Serial.println("Received in Child Node 1: " + json);
  
  if (board_number == 1 && led_status == 1){
    digitalWrite(led, led_status);
    Serial.println("Child Node 1 ON");

  }
  else{
    digitalWrite(led, !led_status);
    //Serial.println("Child Node 1 OFF");
  }
}
Task taskSendMessage( TASK_SECOND * 5, TASK_FOREVER, &sendMessage );

void sendMessage()
{
  msg1 = "Hello from Child Node 1";
  //mesh.sendBroadcast(msg1);
  DynamicJsonDocument doc(1024);
  // Tempature 
  //json doc
  doc["type"] = "Data";
  sensors.requestTemperatures(); 

  //Serial.print("Celsius temperature: ");
  //Serial.print(sensors.getTempCByIndex(0)); 
 
 
  double temp = sensors.getTempCByIndex(0);
  temp1=round(temp*100)/100.0;
  ph=round(ph*100)/100.0;
  doc["child1_temperature"] = temp1;
  doc["child1_ph"] = ph;
  doc["Node Name"] = nodeName;
  doc["msg1"] = msg1;
  doc["led_status"] = led_status;
  String msg ;
  serializeJson(doc, msg);
  mesh.sendBroadcast( msg );
  //Serial.println("");
  //Serial.println("Mesh Broadcast Sensor Data from Child Node - " + msg);
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
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT); 
  digitalWrite(LED_PIN,LOW);
  sensors.begin();
  pinMode(potPin, INPUT);
  delay(1000);

  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP | CONNECTION );  // set before init() so that you can see startup messages
  // mesh.setDebugMsgTypes( ERROR | STARTUP | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  Serial.println("\n");
  mesh.onReceive(&receivedCallback);
  Serial.println("\n");
  mesh.onNewConnection(&newConnectionCallback);
  Serial.println("\n");
  mesh.onChangedConnections(&changedConnectionCallback);
  Serial.println("\n");
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  Serial.println("\n");
  userScheduler.addTask( taskSendMessage );
  Serial.println("\n");
  taskSendMessage.enable();
}

// Other code remains unchanged

void readSensor() {
  Value = analogRead(potPin);

  // Convert analog reading to voltage
  float voltage = Value * (3.3 / 4095.0);
  Serial.print("Temperature-1 : ");
  Serial.print(String(temp1));

  // Convert voltage to pH using the Nernst equation
  // pH = slope * voltage + intercept
  float slope = -6.35; // Adjust based on your calibration
  float intercept = 21.34; // Adjust based on your calibration
  ph = slope * voltage + intercept;

  Serial.print(" | pH: ");
  Serial.println(ph, 2);
}

void loop() {

  readSensor();

  // Your other mesh-related code
  mesh.update();
  delay(1000);
}