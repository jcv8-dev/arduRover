# arduRover
An ArduPilot powered sonar vessel


## Parts
- Omnibus F4 V3S Flight Controller
- FlySky FS-i6X RC Transmitter
- FlySky FS-X6B RC Receiver
- A0221A4 Sonar (Signal needs to be translated with an ESP8266, see ESP8266_Echolot.ino)



### Flightcontroller
#### Serial Ports mapping
- RX6/TX6 -> SERIAL4


### Sonar
The sonar I have isn't supported by ArduPilot. 
So naturally i wrote a simple Arduino Sketch which runs on a Lolin(Wemos) D1 mini. It translates any value you can calculate or read from any pins to the protocol of the GY-US42v2 RangeFinder. Connect the D1 to SERIAL4 and configure the rangefinder to be a GY-US42v2.

The A0221A4 has a range of 3-450cm in air. In water this calculates to about 14-1900cm. Set the rangefinder min/max accordingly.
Since the Sonar outputs a value in cm and i have no clue which speed of sound it uses to calculate the distance, the distance in water is not as accurate as it could be. I used (1 / 343) * 1481 = 4,3177842566 as a scaling factor for the distance in air, to get the distance in water.


### Problems
I have found the Omnibus F4 V3S to have too few Serial Ports for my needs. When I connect the Sonar, the RC Receiver stops communicating with the Flightcontroller, indicating the RX input and RX6/TX6 (i.e. Serial4) use the same UART. I am experimenting with logging the depth on the translating ESP8266 and grabbing the position from the gps by tapping into the antennnas tx and writing a csv to a microsd on a daughter board. This might be even simpler than extracting depth and position from the ArduPilot logs.
