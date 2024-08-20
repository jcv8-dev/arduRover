#include <SoftwareSerial.h>
#include <stdarg.h>

#define LOGGING                    // Define LOGGING to enable logging through serial out
#define SERIAL_OUT_BAUD_RATE 9600  // serial monitor baud rate for logging

#define FC_BAUD_RATE 9600  // flight controller serial baud rate
#define FC_RX D1           // flight controller serial rx
#define FC_TX D2           // flight controller serial tx
#define FC_COMM_LENGTH 7   // number of bytes in fc data communication

#define SONAR_BAUD_RATE 9600  // sonar serial baud rate
#define SONAR_RX D5           // sonar serial rx
#define SONAR_TX D6           // sonar serial tx
#define SONAR_TRN_PIN D7      // sonar transmission begin pin
#define SONAR_COMM_LENGTH 8   // number of bytes in sonar communication


#define TIMEOUT_GET_ADDRESS_MS 10000
#define TIMEOUT_GET_DATA_MS 5000
#define REFRESH_RATE_MS 1000

#define TO_CM(dist_mm) (dist_mm * 0.1)
#define TO_DIST_WATER(dist_air) (dist_air * (1 / 343.0) * 1481)  // convert speed of sound through air to speed of sound through water

SoftwareSerial fc(FC_RX, FC_TX);
SoftwareSerial sonar(SONAR_RX, SONAR_TX);

uint data_to_fc[FC_COMM_LENGTH] = {
  // GY-US12V2 Protocol Constants
  0x5A,
  0x5A,
  0x45,
  0x02,
  // GY-US12V2 Protocol Variables
  0x00,
  0x00,
  0x00,
};

byte sonar_get_address_req[SONAR_COMM_LENGTH] = { 0xFF, 0x03, 0x02, 0x00, 0x00, 0x01, 0x90, 0x6C };
byte sonar_get_distance_req[SONAR_COMM_LENGTH] = { 0x01, 0x03, 0x01, 0x00, 0x00, 0x01, 0x85, 0xF6 };

void setup() {
  fc.begin(FC_BAUD_RATE);
  sonar.begin(SONAR_BAUD_RATE);
  Serial.begin(SERIAL_OUT_BAUD_RATE);

  pinMode(SONAR_TRN_PIN, OUTPUT);

  sendDataToSonar(sonar_get_address_req);
  receiveDataFromSonar(TIMEOUT_GET_ADDRESS_MS);
}

void loop() {
  uint distance = getDistanceCM();
  sendDistToFC(distance);
  delay(REFRESH_RATE_MS);
}

uint getDistanceCM() {
  sendDataToSonar(sonar_get_distance_req);
  uint distance = receiveDataFromSonar(TIMEOUT_GET_DATA_MS);

  double distance_air_cm = TO_CM(distance);
  double distance_water_cm = TO_DIST_WATER(distance_air_cm);

  log("Air:\t%.2f cm\n", distance_air_cm);
  log("Water\t%.2f cm\n\n", distance_water_cm);

  return distance;
}

void sendDistToFC(uint distance) {
  data_to_fc[FC_COMM_LENGTH - 3] = distance >> (FC_COMM_LENGTH + 1);                // Shift right for high byte
  data_to_fc[FC_COMM_LENGTH - 2] = distance & 0xFF;                                 // bitmask for low byte
  data_to_fc[FC_COMM_LENGTH - 1] = lsb_checksum(data_to_fc, (FC_COMM_LENGTH - 1));  // calculate checksum
  sendDataToFC(data_to_fc);
}

/*
Calculate least significant bit checksum
*/
uint lsb_checksum(uint* data, uint size) {
  uint sum = 0;
  for (int i = 0; i < size; i++) {
    sum += data[i];  // add all values
  }
  return sum & 0xFF;  // return last byte
}

void sendDataToFC(uint* data) {
  for (int i = 0; i < 7; i++) {
    fc.write(data_to_fc[i]);
  }
}

void sendDataToSonar(byte* data) {
  digitalWrite(SONAR_TRN_PIN, 1);
  sonar.write(data, SONAR_COMM_LENGTH);
  sonar.flush();
  digitalWrite(SONAR_TRN_PIN, 0);
}

#define NO_NEW_SONAR_DATA_AVAILABLE 0

uint receiveDataFromSonar(unsigned long timeout_ms) {
  static uint latest_data = 0;
  unsigned long initial_timestamp = millis();

  do {
    delay(10);
    if (millis() - initial_timestamp > timeout_ms) {
      log("Request Timed Out...\n");
      break;
    }
  } while (sonar.available() == NO_NEW_SONAR_DATA_AVAILABLE);

  if (sonar.available() > NO_NEW_SONAR_DATA_AVAILABLE) {
    byte data[SONAR_COMM_LENGTH] = {
      0x00,
      0x00,
      0x00,
      0x00,
      0x00,
      0x00,
      0x00,
    };
    sonar.readBytes(data, 7);
    latest_data = word(data[3], data[4]);
  }

  return latest_data;
}

void log(const char* format, ...) {
#ifdef LOGGING
  char buffer[128];  // Buffer to store the formatted string
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Serial.print(buffer);
#endif
}

/*
  Noise, for testing without a sonar connected
*/
uint noise() {
  return rand() % 20;
}
