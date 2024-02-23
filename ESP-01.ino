#include <Adafruit_Sensor.h>
#include "painlessMesh.h"
#include <Arduino_JSON.h>
#include <Servo.h>

// MESH Details
#define   MESH_PREFIX     "RNTMESH" //name for your MESH
#define   MESH_PASSWORD   "MESHpassword" //password for your MESH
#define   MESH_PORT       5555 //default port

//Number for this node
int nodeNumber = 1;
int node1 = 0;
String a;

const int rmotor1Pin1 = 14;
const int rmotor1Pin2 = 27;
const int rmotor2Pin1 = 13;
const int rmotor2Pin2 = 12;

const int lmotor1Pin1 = 33;
const int lmotor1Pin2 = 32;
const int lmotor2Pin1 = 26;
const int lmotor2Pin2 = 25;

const int rEN1Pin1 = 19;
const int rEN1Pin2 = 23;
const int lEN1Pin1 = 5;
const int lEN1Pin2 = 18;

int REncoderAvg = 0;
int LEncoderAvg = 0;

int Encoder1 = 0;
int Encoder2 = 0;
int Encoder3 = 0;
int Encoder4 = 0;

int EN1 = 0;
int EN2 = 0;
int EN3 = 0;
int EN4 = 0;

int pEN1count = 0;
int pEN2count = 0;
int pEN3count = 0;
int pEN4count = 0;

int cEN1count = 0;
int cEN2count = 0;
int cEN3count = 0;
int cEN4count = 0;

int incomingTurn = 0;
int incomingDirection = 0;
int incomingSpeed = 0;

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
  jsonReadings["ROBOT_ESP1_Direction"] = incomingDirection;
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
  if(elev<1000) {  node1 = elev; }
  double Turn = myObject["ROBOT_ESP2_Turn"];
  if(Turn<1000) { incomingTurn = Turn;  }
  double Speed = myObject["ROBOT_ESP2_Speed"];
  if(Speed<1000)  { incomingSpeed = Speed;  }
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
  pinMode(rmotor1Pin1, OUTPUT);
  pinMode(rmotor1Pin2, OUTPUT);
  pinMode(rmotor2Pin1, OUTPUT);
  pinMode(rmotor2Pin2, OUTPUT);

  pinMode(lmotor1Pin1, OUTPUT);
  pinMode(lmotor1Pin2, OUTPUT);
  pinMode(lmotor2Pin1, OUTPUT);
  pinMode(lmotor2Pin2, OUTPUT);

  digitalWrite(lmotor1Pin1, LOW);
  digitalWrite(lmotor1Pin2, LOW);
  digitalWrite(lmotor2Pin1, LOW);
  digitalWrite(lmotor2Pin2, LOW);

  digitalWrite(rmotor1Pin1, LOW);
  digitalWrite(rmotor1Pin2, LOW);
  digitalWrite(rmotor2Pin1, LOW);
  digitalWrite(rmotor2Pin2, LOW);

  pinMode(rEN1Pin1, INPUT);
  pinMode(rEN1Pin2, INPUT);
  pinMode(lEN1Pin1, INPUT);
  pinMode(lEN1Pin2, INPUT);
  
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
    getCMD();
    StartEncoder();
    operatemotor();
    print();
  }
}

void loop() {
  mesh.update();
}


void getCMD()
{
  if(node1 == 0)
  {
    incomingDirection = node1;
  }
  else if(node1>10 && node1<20)
  {
    incomingDirection = node1 - 10;
  }
}

void print()
{
  Serial.print("   Incomming CMD = ");
  Serial.print(node1);
  Serial.print("   Incomming Turn = ");
  Serial.print(incomingTurn);
  Serial.print("   Incomming Speed = ");
  Serial.print(incomingSpeed);
  Serial.print("   Incomming Direction = ");
  Serial.print(incomingDirection);
  Serial.print("   En1 = ");
  Serial.print(EN1);
  Serial.print("   En2 = ");
  Serial.print(EN2);
  Serial.print("   En3 = ");
  Serial.print(EN3);
  Serial.print("   En4 = ");
  Serial.print(EN4);
  Serial.print("   LEncoderAvg = ");
  Serial.print(LEncoderAvg);
  Serial.print("   REncoderAvg = ");
  Serial.print(REncoderAvg);
  Serial.println();
}

void operatemotor()
{
  controldirection();
}

void controldirection()
{
  if(node1 == 0)
  {
    incomingDirection = 0;
    incomingTurn = 0;
    
    digitalWrite(rmotor1Pin1, LOW);
    digitalWrite(rmotor1Pin2, LOW);
    digitalWrite(rmotor2Pin1, LOW);
    digitalWrite(rmotor2Pin2, LOW);

    digitalWrite(lmotor1Pin1, LOW);
    digitalWrite(lmotor1Pin2, LOW);
    digitalWrite(lmotor2Pin1, LOW);
    digitalWrite(lmotor2Pin2, LOW);
  }
  else if(incomingDirection == 5)
  {
    digitalWrite(rmotor1Pin1, LOW);
    digitalWrite(rmotor1Pin2, LOW);
    digitalWrite(rmotor2Pin1, LOW);
    digitalWrite(rmotor2Pin2, LOW);

    digitalWrite(lmotor1Pin1, LOW);
    digitalWrite(lmotor1Pin2, LOW);
    digitalWrite(lmotor2Pin1, LOW);
    digitalWrite(lmotor2Pin2, LOW);
  }
  else if(incomingDirection == 4 && incomingTurn == 5)
  {
    digitalWrite(rmotor1Pin1, HIGH);
    digitalWrite(rmotor1Pin2, LOW);
    digitalWrite(rmotor2Pin1, HIGH);
    digitalWrite(rmotor2Pin2, LOW);

    digitalWrite(lmotor1Pin1, HIGH);
    digitalWrite(lmotor1Pin2, LOW);
    digitalWrite(lmotor2Pin1, HIGH);
    digitalWrite(lmotor2Pin2, LOW);
  }
  else if(incomingDirection == 6 && incomingTurn == 5)
  {
    if(LEncoderAvg<REncoderAvg)
    {
      digitalWrite(rmotor1Pin1, LOW);
      digitalWrite(rmotor1Pin2, LOW);
      digitalWrite(rmotor2Pin1, LOW);
      digitalWrite(rmotor2Pin2, LOW);
      if(EN3<EN4)
      {
        digitalWrite(lmotor1Pin1, LOW);
        digitalWrite(lmotor1Pin2, LOW);
        digitalWrite(lmotor2Pin1, LOW);
        digitalWrite(lmotor2Pin2, HIGH);
      }
      else
      {
        digitalWrite(lmotor1Pin1, LOW);
        digitalWrite(lmotor1Pin2, HIGH);
        digitalWrite(lmotor2Pin1, LOW);
        digitalWrite(lmotor2Pin2, HIGH);
      }
    }
    else
    {
      if(EN2<EN1)
      {
        digitalWrite(rmotor1Pin1, LOW);
        digitalWrite(rmotor1Pin2, LOW);
        digitalWrite(rmotor2Pin1, LOW);
        digitalWrite(rmotor2Pin2, HIGH);
      }
      else
      {
        digitalWrite(rmotor1Pin1, LOW);
        digitalWrite(rmotor1Pin2, HIGH);
        digitalWrite(rmotor2Pin1, LOW);
        digitalWrite(rmotor2Pin2, HIGH);
      }

      if(EN3<EN4)
      {
        digitalWrite(lmotor1Pin1, LOW);
        digitalWrite(lmotor1Pin2, LOW);
        digitalWrite(lmotor2Pin1, LOW);
        digitalWrite(lmotor2Pin2, HIGH);
      }
      else
      {
        digitalWrite(lmotor1Pin1, LOW);
        digitalWrite(lmotor1Pin2, HIGH);
        digitalWrite(lmotor2Pin1, LOW);
        digitalWrite(lmotor2Pin2, HIGH);
      }
    }
  }

  else if(incomingDirection == 6 && incomingTurn == 4)
  {
    digitalWrite(rmotor1Pin1, LOW);
    digitalWrite(rmotor1Pin2, HIGH);
    digitalWrite(rmotor2Pin1, LOW);
    digitalWrite(rmotor2Pin2, HIGH);

    digitalWrite(lmotor1Pin1, LOW);
    digitalWrite(lmotor1Pin2, HIGH);
    digitalWrite(lmotor2Pin1, LOW);
    digitalWrite(lmotor2Pin2, LOW);
  }
  else if(incomingDirection == 6 && incomingTurn == 6)
  {
    digitalWrite(rmotor1Pin1, LOW);
    digitalWrite(rmotor1Pin2, HIGH);
    digitalWrite(rmotor2Pin1, LOW);
    digitalWrite(rmotor2Pin2, LOW);

    digitalWrite(lmotor1Pin1, LOW);
    digitalWrite(lmotor1Pin2, HIGH);
    digitalWrite(lmotor2Pin1, LOW);
    digitalWrite(lmotor2Pin2, HIGH);
  }
  else if(incomingDirection == 7 && incomingTurn == 9)
  {
    digitalWrite(rmotor1Pin1, LOW);
    digitalWrite(rmotor1Pin2, HIGH);
    digitalWrite(rmotor2Pin1, LOW);
    digitalWrite(rmotor2Pin2, HIGH);

    digitalWrite(lmotor1Pin1, HIGH);
    digitalWrite(lmotor1Pin2, LOW);
    digitalWrite(lmotor2Pin1, HIGH);
    digitalWrite(lmotor2Pin2, LOW);
  }
  else if(incomingDirection == 8 && incomingTurn == 9)
  {
    digitalWrite(rmotor1Pin1, HIGH);
    digitalWrite(rmotor1Pin2, LOW);
    digitalWrite(rmotor2Pin1, HIGH);
    digitalWrite(rmotor2Pin2, LOW);

    digitalWrite(lmotor1Pin1, LOW);
    digitalWrite(lmotor1Pin2, HIGH);
    digitalWrite(lmotor2Pin1, LOW);
    digitalWrite(lmotor2Pin2, HIGH);
  }
}

void StartEncoder()
{
  Encoder1 = digitalRead(rEN1Pin1);
  Encoder2 = digitalRead(rEN1Pin2);
  Encoder3 = digitalRead(lEN1Pin1);
  Encoder4 = digitalRead(lEN1Pin2);

  cEN1count = Encoder1;
  cEN2count = Encoder2;
  cEN3count = Encoder3;
  cEN4count = Encoder4;

  if (cEN1count!=pEN1count)
  {
    if(cEN1count==1)
    {
      EN1 = EN1+1;
    }
  }
  else  { EN1 = EN1;  }

  if (cEN2count!=pEN2count)
  {
    if(cEN2count==1)
    {
      EN2 = EN2+1;
    }
  }
  else  { EN2 = EN2;  }

  if (cEN3count!=pEN3count)
  {
    if(cEN3count==1)
    {
      EN3 = EN3+1;
    }
  }
  else  { EN3 = EN3;  }

  if (cEN4count!=pEN4count)
  {
    if(cEN4count==1)
    {
      EN4 = EN4+1;
    }
  }
  else  { EN4 = EN4;  }

  pEN1count = cEN1count;
  pEN2count = cEN2count;
  pEN3count = cEN3count;
  pEN4count = cEN4count;

  REncoderAvg = (EN1+EN2)/2;
  LEncoderAvg = (EN3+EN4)/2;
}
