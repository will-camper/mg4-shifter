# mg4-shifter

Simple Arduino Due program for controlling MG4 Gear selector and reading the current selection

 
# Hardware 
 MG4/SAIC Motor Shift-by-wire 
 Part number 11357121  

Connector: Molex 347290122 
https://www.molex.com/en-us/products/part-detail/0347290122


# Pinout 
(When viewed from the rear of the unit, Pin1 is top-left, Pin12 is bottom right)

| Pin | OEM Wire Colour | Function |
|-----|-----------------|----------|
|1 | Black/Yellow | 12V |
|5 | Pink/White | 12V WakeupEnable |
|7 | Yellow/Blue | PT-CAN Ext High |
|8 | Green/Yellow | PT-CAN Ext Low |
|12 | Black | GND |

# Operation

On poweron, the selector will start in the PARK position, and this will be communicated in message 0x09D

To enable shifting three messages must be sent to the shifter, repeated at 50ms, CanIds: 0x047, 0x12a, 0x2b6 as described below.

Stopping sending the 0x47 message will cause the shifter to enter into PARK position and prevent a gear being selected.

Restarting 0x47 messages will re-enable the shifter and allow a gear to be selected.

# CAN Bus

The Powertrain CAN Bus has a baud rate of 500kBaud.

Most messages have a standard CRC-8-SAE J1850. (polynomial 0x1D)

CAN Messages are sent every 50ms.

Observed data is described below.

| CanId | Message | Direction | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
|-------|---------|-----------|---|---|---|---|---|---|---|---|
| 0x09D | Shifter Status | From Shifter | CRC8 (bytes1-7) | Counter 4-bit | F0-FF | 03 |  POS1 | POS2 | F0 or F1 | 38 or 39 | FF |
| 0x047 | Control Msg1 | "Activate Shifter"? | To Shifter | CRC8 (bytes1-7) | Counter 4-bit | 00-0F | 45 | 9B | 7F | FF | FF | FE or FF |
| 0x12a | Control Msg2 | To Shifter | CRC8 (bytes1-7) | Counter 4-bi F0-FF | FF | FF | FF | FF | FF | 01,11,13,14 |
| 0x2b6 | Control Msg3 | To Shifter | 00 | 00 | 02 | 00 | 00 | 00 | 00 | 00 |

The Gear selection is communicated over CAN, and can be decoded as follows.

POS1 is sufficient to indicate the selected gear.

| Selected Gear | POS1 | POS2 |
|---------------|-----|-----|
| DRIVE | 05 | 0A |
| NEUTRAL | 06 | 09 |
| REVERSE | 07 | 08 |
| PARK | 08 | 07 |

# Further Info

https://openinverter.org/wiki/MG_4_Gear_Selector

