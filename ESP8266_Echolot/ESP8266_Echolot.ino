#include <SoftwareSerial.h>

SoftwareSerial s(D1,D2); //connect flight controller Serial interface to these pins

uint data[7] = {};
uint distance;

void setup(){
  s.begin(9600);
  Serial.begin(9600);
  
  // GY-US12V2 Protocol Constants
  data[0] = 0x5A;
  data[1] = 0x5A; 
  data[2] = 0x45;
  data[3] = 0x02;
}

void loop(){
  distance = getDistance();

  data[4] = distance >> 8; // Shift right for high byte
  data[5] = distance & 0xFF; // bitmask for low byte
  data[6] = checksum(data, 6); //calculate lsb checksum
  Serial.printf("Distance: %d cm\n", distance);
  for(int i = 0; i < 7; i++){
    s.write(data[i]); //send data to flight controller 
  }
  delay(90);
}

/*
Calculate least significant bit checksum
*/
uint checksum(uint* data, uint size){
  uint sum = 0;
  for(int i = 0; i < size; i++){
    sum+=data[i]; // add all values
  }
  return sum & 0xFF; //return last byte
}

/*
Noise, for testing without a sonar connected
*/
uint noise(){
  return rand() % 20;
}

/*
Read any sensor, replace with your own code
*/
uint getDistance(){
  return 0x0123+noise();
}
