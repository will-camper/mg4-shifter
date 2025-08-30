/*
MG4 Shifter Test Code 
Assumes shifter CAN is connected to CAN0 

Code based on sample arduino control programs by Damien Maguire
*/

#include "CRC8.h"
#include "CrcParameters.h"
#include <due_can.h>
#include <arduino-timer.h>

#define SerialDEBUG SerialUSB
template<class T> inline Print &operator<<(Print &obj, T arg) {
  obj.print(arg);
  return obj;
}

CAN_FRAME outframe;  
CAN_FRAME inFrame;   

float Version = 1.0;

auto timer = timer_create_default();


uint8_t can_data[8];
uint8_t can_ctr = 1;
bool activateShifter = true;
bool Display = false;
uint8_t shifterStatus = 0;
CRC8 crc = CRC8(CRC8_SAEJ1850_POLYNOME, CRC8_SAEJ1850_INITIAL, CRC8_SAEJ1850_XOR_OUT, CRC8_SAEJ1850_REV_IN, CRC8_SAEJ1850_REV_OUT);


void setup() {
  SerialDEBUG.begin(115200);
  Can0.begin(CAN_BPS_500K);  // Charger CAN
  Can0.watchFor();
  timer.every(50, sendShifterMessages);
}


void loop() {
  timer.tick();
  checkforinput();
  CheckRecCan();
}


void printCanMessage(CAN_FRAME frame) {
  SerialDEBUG.print(frame.id, 16);
  SerialDEBUG << " : ";
  for (int i = 0; i < 8; ++i) {
    if (frame.data.bytes[i] < 0x10) SerialDEBUG.print('0');
    SerialDEBUG.print(frame.data.bytes[i], 16);
  }
  SerialDEBUG << "\n";
}

void printShifterStatus() {
      switch (shifterStatus) {
       case 0x05:
           SerialDEBUG.println("DRIVE");
           break;
       case 0x06:
           SerialDEBUG.println("NEUTRAL");
           break;
       case 0x07:
           SerialDEBUG.println("REVERSE");
           break;
       case 0x08:
           SerialDEBUG.println("PARK");
           break;
       default:
           SerialDEBUG.print("Unknown status");
           SerialDEBUG << shifterStatus;
           SerialDEBUG << "\n";
      }
}

bool sendShifterMessages(void *unused) {

  can_ctr++;
  if (can_ctr > 0xFF) can_ctr = 0;

  // Control message 1 (shifter activate):
  if (activateShifter) {
    outframe.id = 0x047;
    outframe.length = 8;
    outframe.extended = 0;
    outframe.rtr = 1;
    outframe.data.bytes[1] = can_ctr & 0x0F;
    outframe.data.bytes[2] = 0x45;
    outframe.data.bytes[3] = 0x9b;
    outframe.data.bytes[4] = 0x7F;
    outframe.data.bytes[5] = 0xFF;
    outframe.data.bytes[6] = 0xFF;
    outframe.data.bytes[7] = 0xFF;   
    crc.restart();
    crc.add(&outframe.data.bytes[1], 7);
    outframe.data.bytes[0] = crc.calc();

    Can0.sendFrame(outframe);
  }

  // control message 2
  outframe.id = 0x12a;
  outframe.length = 8;
  outframe.extended = 0;
  outframe.rtr = 1;
  outframe.data.bytes[1] = 0xF0 | (can_ctr & 0x0F);
  outframe.data.bytes[2] = 0xFF;
  outframe.data.bytes[3] = 0xFF;
  outframe.data.bytes[4] = 0xFF;
  outframe.data.bytes[5] = 0xFF;
  outframe.data.bytes[6] = 0xFF;
  outframe.data.bytes[7] = 0x11;   
  crc.restart();
  crc.add(&outframe.data.bytes[1], 7);
  outframe.data.bytes[0] = crc.calc();

  Can0.sendFrame(outframe); 

  // control message 3
  outframe.id = 0x2b6;
  outframe.length = 8;
  outframe.extended = 0;
  outframe.rtr = 1;
  outframe.data.bytes[0] = 0x00;
  outframe.data.bytes[1] = 0x00;
  outframe.data.bytes[2] = 0x02;
  outframe.data.bytes[3] = 0x00;
  outframe.data.bytes[4] = 0x00;
  outframe.data.bytes[5] = 0x00;
  outframe.data.bytes[6] = 0x00;
  outframe.data.bytes[7] = 0x00;   

  Can0.sendFrame(outframe); 
  
  return true;
  
}  


void updateShifterStatus(CAN_FRAME inframe) {
  uint8_t newStatus=inframe.data.bytes[3];
  if (newStatus!=shifterStatus) {
    shifterStatus=newStatus;
    printShifterStatus();
  }
}

void checkforinput() {
  //Checks for keyboard input from Native port
  if (SerialDEBUG.available()) {
    int inByte = SerialDEBUG.read();
    switch (inByte) {
      case 'a':
        activateShifter = !activateShifter;
        SerialDEBUG << "Shifter activation: " << activateShifter << "\n";
        break;
      case 'd':
        SerialDEBUG.println("Debug Deativated");
        Display = !Display;
        SerialDEBUG << "Display CAN received: " << Display << "\n";
        break;
      case '?':  //Print a menu describing these functions
        printMenu();
        break;
    }
  }
}

void printMenu() {
  SerialDEBUG << "\f\n=========== mg4Shifter Version " << Version << " ==============\n";
  printShifterStatus();
  SerialDEBUG << "************ List of Available Commands ************\n\n";
  SerialDEBUG << "  ?  - Print this menu\n ";
  SerialDEBUG << "  a -  Toggle activate Shifter\n";
  SerialDEBUG << "  d -  Toggle display can received\n";
  SerialDEBUG << "**************************************************************\n==============================================================\n\n";
}

void CheckRecCan() {
  if (Can0.available()) {
    Can0.read(inFrame);
    if (inFrame.id==0x09D) {
      updateShifterStatus(inFrame);
    }
    if (Display) {
      SerialDEBUG << "Received:";
      printCanMessage(inFrame);
    }
  }
}
