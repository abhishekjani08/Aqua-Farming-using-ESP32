
// Necessary Libraries
#include "painlessMesh.h"
#include <ArduinoJson.h>
//#include <DHT.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>
#include "GravityTDS.h"

 
// WiFi Credentials
#define   MESH_PREFIX     "whateverYouLike"
#define   MESH_PASSWORD   "somethingSneaky"
#define   MESH_PORT       5555

// Tds Sensor
#define TdsSensorPin A2
GravityTDS gravityTds;
float temperature = 25,tdsValue = 0;


// pH sensor pin
#define SensorPin 0

// Data wire is plugged into port 4
#define ONE_WIRE_BUS 4

// Setup a oneWire instance to communicate with any OneWire devices 
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

int led;
int led_status = 0;
int board_status = 0;

uint32_t raw; // Do sensor
int buf[10]; // pH 



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
Task taskSendMessage( TASK_SECOND * 600 , TASK_FOREVER, &sendMessage );


void sendMessage()
{

  DynamicJsonDocument doc(1024);

  // Do
  raw=analogRead(A1);


  // pH  
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(SensorPin);
    delay(10);
  }
  unsigned long int avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;                      //convert the millivolt into pH value 


  // Tempature 
  sensors.requestTemperatures(); 
  
  Serial.print("Celsius temperature: ");
  Serial.print(sensors.getTempCByIndex(0)); 

  // Tds Sensor 
  temperature = sensors.getTempCByIndex(0);  //add your temperature sensor and read it
  gravityTds.setTemperature(temperature);  // set the temperature and execute temperature compensation
  gravityTds.update();  //sample and calculate
  tdsValue = gravityTds.getTdsValue()

  //json doc
  doc["DO"] = String(raw);
  doc["pH"] = String(phValue);
  doc["Temp"] =  String(sensors.getTempCByIndex(0));
  doc["Tds"] = String(tdsValue);
  
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
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);  //reference voltage on ADC, default 5.0V on Arduino UNO
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();  //initialization
  
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
}
