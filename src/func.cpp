#include "var.h"
#include <i2c_driver.h>
#include <i2c_driver_wire.h>
char lastdata = '0';

// ─── CRC16 (Modbus RTU) ─────────────────────────────────────
uint16_t calcCRC(const uint8_t *buf, int len) {
  uint16_t crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint16_t)buf[pos];
    for (int i = 0; i < 8; i++) {
      if (crc & 1) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

// ─── RS-485 Transceiver Control ─────────────────────────────
void setTransmitMode() {
  digitalWrite(DE_PIN, HIGH);
  digitalWrite(RE_PIN, HIGH);
}

void setReceiveMode() {
  digitalWrite(DE_PIN, LOW);
  digitalWrite(RE_PIN, LOW);
}

void sendFrame(uint8_t *frame, int len) {
  setTransmitMode();
  delayMicroseconds(10);
  RS485.write(frame, len);
  RS485.flush();
  delayMicroseconds(500); 
  setReceiveMode();
}

// ─── GENERIC MODBUS WRITE (0x06) ───────────────────────────
void sendModbusWrite(uint8_t id, uint16_t reg, uint16_t value) {
  uint8_t frame[8];
  frame[0] = id; frame[1] = 0x06;
  frame[2] = reg >> 8; frame[3] = reg & 0xFF;  
  frame[4] = value >> 8; frame[5] = value & 0xFF;
  uint16_t crc = calcCRC(frame, 6);
  frame[6] = crc & 0xFF; frame[7] = crc >> 8;    
  sendFrame(frame, 8);
}
// ─── GENERIC MODBUS READ (0x03) ─────────────────────────────
uint16_t readModbusRegister(uint8_t id, uint16_t reg) {
  // 1. Flush old buffer data
  while(RS485.available()) RS485.read();

  // 2. Construct Read Command
  uint8_t frame[8];
  frame[0] = id;        
  frame[1] = 0x03;      
  frame[2] = reg >> 8;  
  frame[3] = reg & 0xFF;
  frame[4] = 0x00;      
  frame[5] = 0x01;      
  uint16_t crc = calcCRC(frame, 6);
  frame[6] = crc & 0xFF;
  frame[7] = crc >> 8;

  // 3. Send
  sendFrame(frame, 8);

  // 4. Wait for Response (Increased to 200ms)
  uint32_t t = millis();
  while(RS485.available() < 7) {
    // FIX: Increased timeout from 50ms to 200ms
    if(millis() - t > 200) return 0xFFFF; 
  }

  // 5. Read Response
  uint8_t buf[8];
  RS485.readBytes(buf, 7); 

  // 6. Return Value
  return (buf[3] << 8) | buf[4];
}

// ─── CHECK ALARMS ──────────────────────────────────────────
void checkSystemAlarms() {
  uint8_t ids[] = {0x01, 0x02};

  Serial.println("\n--- DIAGNOSTIC CHECK ---");
  for(int i=0; i<2; i++) {
    // FIX: Add delay before talking to the next drive to clear the bus
    if (i > 0) delay(50); 
    
    uint16_t alarm = readModbusRegister(ids[i], 0x2203);
    
    Serial.printf("Drive 0x%02X: ", ids[i]);
    
    if(alarm == 0xFFFF) {
      Serial.println("TIMEOUT (No connection)");
    } else if (alarm == 0) {
      Serial.println("OK");
    } else {
      Serial.printf("!! FAULT !! Code: 0x%04X\n", alarm);
    }
  }
  Serial.println("------------------------\n");
}
// ─── Existing Logic ────────────────────────────────────────

void triggerPathIndividual1(uint8_t path){
  uint16_t value = 0x10 + path;
  Serial.printf(">> Path1 %d\n", path);
  sendModbusWrite(0x01, 0x6002, value);
  if(valdelay == true){
  delay(50);
  }
}
void triggerPathIndividual2(uint8_t path){
  uint16_t value = 0x10 + path;
  Serial.printf(">> Path1 %d\n", path);
  sendModbusWrite(0x02, 0x6002, value);
  if(valdelay == true){
  delay(50);
  }
}
void triggerPathBroadcast(uint8_t path) {
  uint16_t value = 0x10 + path;
  Serial.printf(">> Path %d\n", path);
  sendModbusWrite(0x00, 0x6002, value);
}

void emergencyStopBroadcast() {
  Serial.println(">> E-STOP");
  sendModbusWrite(0x00, 0x6002, 0x0040);
}

void resetSystem() {
  Serial.println("\n--- INITIATING SYSTEM RESET ---");
  emergencyStopBroadcast();
  delay(200);

  uint8_t drives[] = {0x01, 0x02};
  for(int i=0; i<2; i++) {
    uint8_t id = drives[i];
    Serial.printf("Resetting Drive 0x%02X...\n", id);
    sendModbusWrite(id, 0x6000, 0x0000); // Disable
    delay(100);
    sendModbusWrite(id, 0x1801, 0x1111); // Clr Current
    delay(500);
    sendModbusWrite(id, 0x1801, 0x1122); // Clr History
    delay(100);
    sendModbusWrite(id, 0x6000, 0x000F); // Enable
    delay(100);
  }
  Serial.println("--- RESET COMPLETE ---\n");
}

void receiveEvent(int bytesReceived) {
    char _data = kire.read(); 
   // char _data = char(__data);
    //Serial.print(__data);
    Serial.print(_data);
     if(last_data != _data)
    {
      valdelay=true;
      Serial.print("star---------");
    }
    commandTransition(lastdata, _data); //// changes------------
    if(_data == 'a') {
        if (data != '3' || data != '4' || data != '0') {
            rpmAlter = !rpmAlter; 
      } } /*else if(_data == 'b') {
        if (data != '1' || data != '2' || data != '0') {
            rpmAlter_T = !rpmAlter_T; 
      } }*/ else if(_data == 'A') {
        //resetFunc();
         
         triggerPathBroadcast(9);
        data = '0';
      } else if(_data == 'X') {
        //resetFunc();
         //emergencyStopBroadcast();
        // triggerPathBroadcast(9);
         emergencyStopBroadcast();
        data = '0';
      }else if(_data == 'B') {
        //resetFunc();
        resetSystem(); 
        data = '0';    
        startTimeControlCounter = millis(); 
      }else if (_data == 'Y'){
      emergency =!emergency;
      } 
      else if(_data != 10) { 
        /*if((last_data == '1'|| last_data =='2') && (_data == '3' || _data == '4')){
      Serial.print("True. stop ,,,,");
       stop = true; 
    }else */
   
    if((last_data == '3'|| last_data =='4') && (_data == '1' || _data == '2')){
      Serial.print("True. stop ,,,,");
       stop = true; 
    }else if((last_data == '3'|| last_data =='4') && (_data == '0')){
      Serial.print("True. stop ,,,,");
       stop = true; 
    }else if((last_data == '5'|| last_data =='6') && (_data == '1' || _data == '2')){
      Serial.print("True. stop ,,,,");
       acc_no_delay = true; 
    }else if((last_data == '7'|| last_data =='8') && (_data == '1' || _data == '2')){
      Serial.print("True. stop ,,,,");
       acc_no_delay = true; 
    }
    last_data = _data;
          startTimeControlCounter = millis();
          data = _data;
          motion((char)_data);
          //startTimeControlCounter = currentTimeControlCounter;
        }
    }




uint32_t getTeensySerial() {
	uint32_t num;
	num = HW_OCOTP_MAC0 & 0xFFFFFF;
	if (num < 10000000) num = num * 10;
    return num;
}

void scanModbusIDs() {
  int count =0;
  Serial.println("\n--- SCANNING MODBUS IDs (1-10) ---");

  for (uint8_t i = 1; i <= 10; i++) {
    Serial.printf("Pinging ID 0x%02X... ", i);
    // Try to read status register 0x6041
    uint16_t result = readModbusRegister(i, 0x6041);
    
    if (result != 0xFFFF) {
       Serial.println("FOUND! ✅");
       count +=1;
    } else {
       Serial.println("...");
    }
    delay(50);
   
}
  Serial.println("--- SCAN COMPLETE ---\n");
}

void commandTransition(char last_data, char _data){
    if((last_data == '1'|| last_data =='2') && (_data == '3' || _data == '4')){
      Serial.print("True. stop ,,,,");
       stop = true; 
    }else if((last_data == '3'|| last_data =='4') && (_data == '1' || _data == '2')){
      Serial.print("True. stop ,,,,");
       stop = true; 
    }else if((last_data == '5'|| last_data =='6') && (_data == '1' || _data == '2')){
      Serial.print("True. stop ,,,,");
       acc_no_delay = true; 
    }else if((last_data == '7'|| last_data =='8') && (_data == '1' || _data == '2')){
      Serial.print("True. stop ,,,,");
       acc_no_delay = true; 
    }
    last_data = _data;
};