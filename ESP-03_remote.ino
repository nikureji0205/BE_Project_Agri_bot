#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel pixels(16, 13, NEO_GRB + NEO_KHZ800);

// MESH Details
#define   MESH_PREFIX     "RNTMESH" //name for your MESH
#define   MESH_PASSWORD   "MESHpassword" //password for your MESH
#define   MESH_PORT       5555 //default port

//Number for this node
int nodeNumber = 3;
int strint;
int movE = 0;
String a;

int eCount = 0;

//String to send to other nodes with readings
String readings;

Scheduler userScheduler; // to control your personal task
painlessMesh  mesh;

// User stub
void sendMessage() ; // Prototype so PlatformIO doesn't complain
String getReadings(); // Prototype for sending sensor readings

TaskHandle_t Task1;
TaskHandle_t Task2;
//Create tasks: to send messages and get readings;
Task taskSendMessage(TASK_SECOND , TASK_FOREVER, &sendMessage);

String getReadings () {
  JSONVar jsonReadings;
  jsonReadings["Node"] = nodeNumber;
  jsonReadings["CMD"] = strint;
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ){
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["node"];

  double countE = myObject["ROBOT_ESP1"];
  eCount = countE;
}

void newConnectionCallback(uint32_t nodeId){
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback(){
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset){
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup()
{
  Serial.begin(115200);
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
  xTaskCreatePinnedToCore(Task2code,"Task2",10000,NULL,1,&Task2,1);
  pixels.begin();
  pixels.setBrightness(255);
  delay(100);
}

void Task2code( void * pvParameters )
{
  for(;;)
  {
    if(Serial.available( ) > 0)
    { 
      a = Serial.readString( );
      strint = a.toInt();
      pixels.clear();
      pixels.setPixelColor(3, pixels.Color(255, 50, 0));
      pixels.show();
    } 
  }
}

void loop() {
  mesh.update();
}