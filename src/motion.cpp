#include "var.h"
#include <Arduino.h>

void motion(char _data) {
  if(_data == '0') { 
    // Idle state, do nothing
    triggerPathBroadcast(0); 
    rpmAlter_T = false;
    rpmAlter = false;
    return;
  } 
  
  // Debug output -- test for github
  Serial.print("Motion Cmd: ");
  Serial.println(_data);

  if (_data == '1') {
    rpmAlter_T = false;
    if(stop== false){
    if(!rpmAlter)      triggerPathBroadcast(1);
    else               triggerPathBroadcast(10);
    } else if(stop == true)
    {
     triggerPathBroadcast(9);
     delay(200);
     stop = false;
    }
    if (acc_no_delay == true)
    {
        triggerPathBroadcast(10);
        delay(300);
        acc_no_delay = false;
    }
    valdelay =false;
   } else if(_data == '2') {
    rpmAlter_T = false;
    if(stop== false && acc_no_delay == false){
    if(!rpmAlter)      triggerPathBroadcast(2);
    else               triggerPathBroadcast(11);
    } else if(stop == true )
    {
     triggerPathBroadcast(9);
     delay(200);
     stop = false;
    }
    if (acc_no_delay == true)
    {
        triggerPathBroadcast(13);
        delay(300);
        acc_no_delay = false;
    }
    valdelay =false;
  } else if (_data == '3') {
    rpmAlter = false;
    if(stop == true)
    {
        triggerPathBroadcast(9);
        delay(200);
        stop = false;
    }
    triggerPathIndividual1(3);
    triggerPathIndividual2(3);
    valdelay =false;  
  } else if (_data == '4') {
    rpmAlter = false;
     if(stop == true)
    {
        triggerPathBroadcast(9);
        delay(200);
        stop = false;
    }
    triggerPathBroadcast(4); // Fixed copy-paste error (was 3)   
    valdelay =false;
  }  else if(_data == '5') {
    rpmAlter_T = false;
    triggerPathBroadcast(5);
    
  } else if(_data == '6') {
    rpmAlter_T = false;
    triggerPathBroadcast(6);
    
  } else if(_data == '7') {
    rpmAlter_T = false; 
    triggerPathBroadcast(7);
    
  } else if(_data == '8') {
    rpmAlter_T = false;
    triggerPathBroadcast(8); // Fixed logic (was 7)
  } else if(_data == '0') {
   triggerPathBroadcast(0); 
   rpmAlter = false;
   rpmAlter_T = false;
  } 
}