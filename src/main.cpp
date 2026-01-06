#include "var.h"
#include <i2c_driver.h>
#include <i2c_driver_wire.h>

int SLAVE_ADDRESS = 0x72;
bool valdelay = false;
// Timer Variables
float timeConstantControlCounter = 500; 
float startTimeControlCounter = 0;
float elaspedTimeControlCounter = 0;
float currentTimeControlCounter = 0;
bool leftStop = false;
bool rightStop= false;
// State Variables
char data = 'x';
char last_data = '0';
bool rpmAlter_T = 0;
bool rpmAlter = 0;
int path = 0;    
bool printAlter = false;
bool stop = false;
bool acc_no_delay = false;
bool dataStatus = false;

int off = 22;
bool emergency;

void setup() {
  pinMode(DE_PIN, OUTPUT);
  pinMode(RE_PIN, OUTPUT);
  pinMode(off, OUTPUT);
  kire.begin(SLAVE_ADDRESS); 
  setReceiveMode(); // RS485 transmission
  Serial.begin(115200);          
  RS485.begin(38400, SERIAL_8N2);

  Serial.println("System Ready.");
  Serial.println("Press 'c' to Reset Alarms.");
  Serial.println("Press 'k' to Check for Errors.");
  scanModbusIDs();
}

void loop() {
  if(emergency ==true){
    digitalWrite(off, HIGH);
  }else if(emergency == false){
    digitalWrite(off, LOW);
  }
  kire.onReceive(receiveEvent); 
  currentTimeControlCounter = millis(); 
  elaspedTimeControlCounter = currentTimeControlCounter - startTimeControlCounter;

  if(Serial.available()) {
      int _data = Serial.read();
       if(last_data != _data){
      valdelay=true;
      Serial.print("star---------");
    }
      startTimeControlCounter = currentTimeControlCounter; 
      commandTransition(last_data,_data);
      
      if(_data == 'm') {
         Serial.print(getTeensySerial());
         Serial.println(" | Motion Module");
         data = '0'; 
      } 
      else if(_data == 'p') printAlter = !printAlter;
      else if(_data == 'X') {
        emergencyStopBroadcast();
        data = '0';
      }
      else if(_data == 'B') {
        resetSystem(); 
        data = '0';    
        startTimeControlCounter = millis(); 
      }
      // ─── NEW CHECK COMMAND ───
      else if(_data == 'K') {
        checkSystemAlarms();
      }
      else if (_data == 'Y'){
      emergency =!emergency;
      }
      // ─────────────────────────
      else if(_data == 'a') {
        if (data != '3' && data != '4') {
            rpmAlter = !rpmAlter; 
            Serial.print("RPM Mode: "); Serial.println(rpmAlter);
        } 
      } 
      else if(_data == 'b') {
        if (data != '1' && data != '2') {
            rpmAlter_T = !rpmAlter_T; 
            Serial.print("RPM_T Mode: "); Serial.println(rpmAlter_T);
        } 
      } 
      else if(_data != 10 && _data != 13) { 
          data = (char)_data;
          motion(data); 
      }  
  }
  if(elaspedTimeControlCounter > timeConstantControlCounter) {
      if (data != '9' && data != '0') { 
          Serial.println("!! TIMEOUT - BRAKING !!");
          data = '0';  
          motion(data);
      }
      startTimeControlCounter = currentTimeControlCounter;
  }
}