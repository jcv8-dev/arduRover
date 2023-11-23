#include <SoftwareSerial.h>

SoftwareSerial fc(D1,D2); //connect flight controller Serial interface to these pins
SoftwareSerial sonar(D5,D6);

uint data[7] = {};
uint distance;
uint distance_water;

byte GET_ADDRESS[8] = {0xFF, 0x03, 0x02, 0x00, 0x00, 0x01, 0x90, 0x6C};
byte GET_DISTANCE[8] = {0x01, 0x03, 0x01, 0x00, 0x00, 0x01, 0x85, 0xF6};

#define TRN_PIN D7

#define TIMEOUT_DELAY 5000

unsigned long time_ = 0;


void setup(){
  fc.begin(9600);
  sonar.begin(9600);
  Serial.begin(9600);
  pinMode(TRN_PIN, OUTPUT);
  sendData(GET_ADDRESS);
  receiveData();
  // GY-US12V2 Protocol Constants
  data[0] = 0x5A;
  data[1] = 0x5A; 
  data[2] = 0x45;
  data[3] = 0x02;
}

void loop(){
  uint distance_ = getDistance()/10;
  Serial.printf("Return %d\n", distance_);
  data[4] = distance >> 8; // Shift right for high byte
  data[5] = distance & 0xFF; // bitmask for low byte
  data[6] = checksum(data, 6); //calculate lsb checksum
  //Serial.printf("Distance: %d cm\n", distance);
  for(int i = 0; i < 7; i++){
    fc.write(data[i]); //send data to flight controller 
  }
  delay(100);
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
  sendData(GET_DISTANCE);
  receiveData();
  return distance_water;
}

void sendData(byte *data)
{
    time_ = millis();
    digitalWrite(TRN_PIN, 1);
    for (int i = 0; i < 8; i++)
    {
    Serial.flush();
    sonar.write(data, 8);
    sonar.flush();
    digitalWrite(TRN_PIN, 0);
    delay(10);
    }
}

void receiveData()
{
    byte data[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    while (sonar.available() == 0)
    {
        if (millis() - time_ > TIMEOUT_DELAY)
        {
          Serial.println("Request Timed Out...");
          break;
        }
    }
    if (sonar.available() > 0)
    {
        sonar.readBytes(data, 7);
        distance = word(data[3],data[4]);
        Serial.printf("Air:\t%f\n",distance*0.1f);
        distance_water = distance * 4.353f; // convert to speed of sound in water
        Serial.printf("Water\t%f\n", distance_water*0.1f);
    }
    delay(10);
}
