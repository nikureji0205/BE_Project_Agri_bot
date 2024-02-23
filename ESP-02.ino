#include <Adafruit_Sensor.h>
#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <Servo.h>

// MESH Details
#define   MESH_PREFIX     "RNTMESH" //name for your MESH
#define   MESH_PASSWORD   "MESHpassword" //password for your MESH
#define   MESH_PORT       5555 //default port

//Number for this node
int nodeNumber = 2;
int node1 = 0;
String a;

int count1 = 3000;

const int rmotorPin1 = 19;
const int rmotorPin2 = 21;
const int lmotorPin1 = 23;
const int lmotorPin2 = 22;

const int fservoPin1 = 27;
const int cservoPin1 = 14;
const int lservoPin1 = 25;
const int lservoPin2 = 26;
const int rservoPin1 = 32;
const int rservoPin2 = 33;

int buttonState = 0;

int fposDegrees1 = 90;
int cposDegrees1 = 90;
int lposDegrees1 = 90;
int lposDegrees2 = 90;
int rposDegrees1 = 90;
int rposDegrees2 = 90;

int incomingTurn;
int incomingDirection;
int incomingSpeed;

Servo fservo1;
Servo cservo1;
Servo lservo1;
Servo lservo2;
Servo rservo1;
Servo rservo2;

//String to send to other nodes with sensor readings
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
  jsonReadings["node"] = nodeNumber;
  jsonReadings["ROBOT_ESP2_Turn"] = incomingTurn;
  jsonReadings["ROBOT_ESP2_Speed"] = incomingSpeed;
  readings = JSON.stringify(jsonReadings);
  return readings;
}

void sendMessage () {
  String msg = getReadings();
  mesh.sendBroadcast(msg);
}

//Init BME280

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  JSONVar myObject = JSON.parse(msg.c_str());
  int node = myObject["Node"];
  double elev = myObject["CMD"];
  if(elev<1000)
  {
    node1 = elev;
  }
  double Direction = myObject["ROBOT_ESP2_Direction"];
  if(Direction<1000)
  {
    incomingDirection = Direction;
  }
}

void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
}

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);
  fservo1.attach(fservoPin1);
  cservo1.attach(cservoPin1);
  lservo1.attach(lservoPin1);
  lservo2.attach(lservoPin2);
  rservo1.attach(rservoPin1);
  rservo2.attach(rservoPin2);
  pinMode(rmotorPin1, OUTPUT);
  pinMode(rmotorPin2, OUTPUT);
  pinMode(lmotorPin1, OUTPUT);
  pinMode(lmotorPin2, OUTPUT);
  
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | STARTUP );  // set before init() so that you can see startup messages........18602662666

  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT );
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();

  xTaskCreatePinnedToCore(Task2code,"Task2",10000,NULL,1,&Task2,1);
  delay(100);
}


//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters )
{
  for(;;)
  {
    getDistance();
    if(node1 == 0)
    {
      incomingSpeed = node1;
      incomingTurn = node1;
    }
    if(node1 > 0 && node1 < 10)
    {
      incomingSpeed = node1;
    }
    if(node1>20 && node1<30)
    {
      incomingTurn = node1-20;
    }
    operatemotor();
    Serial.print("   Incomming CMD = ");
    Serial.print(node1);
    Serial.print("   Incomming Turn = ");
    Serial.print(incomingTurn);
    Serial.print("   Incomming Speed = ");
    Serial.print(incomingSpeed);
    Serial.print("   Incomming Direction = ");
    Serial.print(incomingDirection);
    Serial.println();
  }
}

void loop() {
  mesh.update();
}

void getDistance()
{
  count1 = 10;
}


void operatemotor()
{
  controlapplication();
  controlturn();
}

void controlapplication()
{
  if(incomingSpeed == 1)
  {
    fposDegrees1 = 45;
    cposDegrees1 = 45;
    fservo1.write(fposDegrees1);
    cservo1.write(cposDegrees1);
  }
  else if(incomingSpeed == 2)
  {
    cposDegrees1 = 45;
    cservo1.write(cposDegrees1);
  }
  else if(incomingSpeed == 3)
  {
    cposDegrees1 = 120;
    cservo1.write(cposDegrees1);
  }
  else if(incomingSpeed == 4)
  {
    fposDegrees1 = 45;
    fservo1.write(fposDegrees1);
  }
  else if(incomingSpeed == 5)
  {
    fposDegrees1 = 120;
    fservo1.write(fposDegrees1);
  }
}

void controlturn()
{
  if(incomingSpeed == 0)
  {
    incomingDirection = 0;
    incomingTurn = 0;
    fposDegrees1 = 45; cposDegrees1 = 45;
    
    lposDegrees1 = 88;  rposDegrees1 = 98;
    lposDegrees2 = 87;  rposDegrees2 = 89;
    
    lservo1.write(lposDegrees1);
    lservo2.write(lposDegrees2);
    rservo1.write(rposDegrees1);
    rservo2.write(rposDegrees2);
    fservo1.write(fposDegrees1);
    cservo1.write(cposDegrees1);
  }
  else if(incomingTurn == 5)
  {
    lposDegrees1 = 85;  rposDegrees1 = 96;
    lposDegrees2 = 87;  rposDegrees2 = 87;
    
    lservo1.write(lposDegrees1);
    lservo2.write(lposDegrees2);
    rservo1.write(rposDegrees1);
    rservo2.write(rposDegrees2);
  }
  else if(incomingTurn == 4)
  {
    lposDegrees1 = 25;  rposDegrees1 = 45;
    lposDegrees2 = 90;  rposDegrees2 = 90;
    
    lservo1.write(lposDegrees1);
    lservo2.write(lposDegrees2);
    rservo1.write(rposDegrees1);
    rservo2.write(rposDegrees2);
  }
  else if(incomingTurn == 6)
  {
    lposDegrees1 = 120; rposDegrees1 = 155;
    lposDegrees2 = 90;  rposDegrees2 = 90;
    
    lservo1.write(lposDegrees1);
    lservo2.write(lposDegrees2);
    rservo1.write(rposDegrees1);
    rservo2.write(rposDegrees2);
  }
  else if(incomingTurn == 9)
  {
    lposDegrees1 = 130; rposDegrees1 = 35;
    lposDegrees2 = 35;  rposDegrees2 = 130;
    
    lservo1.write(lposDegrees1);
    lservo2.write(lposDegrees2);
    rservo1.write(rposDegrees1);
    rservo2.write(rposDegrees2);
  }
}