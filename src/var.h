#pragma once
#include <Arduino.h>

// ─── Hardware Configuration ──────────────────────────────────
#define RS485 Serial2    
#define DE_PIN 2
#define RE_PIN 3

// ─── Global Variables ────────────────────────────────────────
extern char data;
extern int path;

extern int off;  
extern bool valdelay; 
extern bool rpmAlter; 
extern bool rpmAlter_T;
extern bool stop ; 
extern bool acc_no_delay;
extern bool dataStatus;
extern bool leftStop;
extern bool rightStop;
// ─── Timer Variables ─────────────────────────────────────────
extern float startTimeControlCounter; 
extern float elaspedTimeControlCounter;
extern float currentTimeControlCounter;
extern float timeConstantControlCounter;
extern char last_data;
extern bool emergency;
// ─── Function Prototypes ─────────────────────────────────────
uint16_t calcCRC(const uint8_t *buf, int len);
void setTransmitMode();
void setReceiveMode();
void sendFrame(uint8_t *frame, int len);

// Modbus Commands
void sendModbusWrite(uint8_t id, uint16_t reg, uint16_t value);
uint16_t readModbusRegister(uint8_t id, uint16_t reg); // <--- NEW READ FUNCTION
void checkSystemAlarms();                              // <--- NEW CHECK FUNCTION

void triggerPathIndividual1(uint8_t path);
void triggerPathIndividual2(uint8_t path);
void triggerPathBroadcast(uint8_t path);
void emergencyStopBroadcast();
void resetSystem(); 

// System Logic
void receiveEvent(int bytesReceived);
uint32_t getTeensySerial();
void motion(char _data);
void scanModbusIDs(); // Add prototype
void commandTransition(char last_data, char _data);