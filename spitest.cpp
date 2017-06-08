// Used wiringPiSpi library demo -- see below.
// Demo code taken from https://github.com/sparkfun/Pi_Wedge -- thanks to Byron Jacquot @ SparkFun Electronics
// WiringPiSpi library here: http://wiringpi.com/download-and-install/

#include <iostream>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <atomic>
#include <chrono>
#include <thread>

using namespace std;

// channel is the wiringPi name for the chip select (or chip enable) pin.
// Set this to 0 or 1, depending on how it's connected.
static const int CHANNEL = 0;

#define PACKET_BODY_LENGTH 2
bool transmitMany = false;
int bytesSent = 0;
int CHIPSELECT = 16;
int incomingByte = 0;
int numError = 0;

#define RESPONSE_OK 0
#define RESPONSE_BAD_PACKET 1
#define RESPONSE_IMU_DATA 2
#define RESPONSE_MIRROR_DATA 3
#define RESPONSE_ADCS_REQUEST 4

uint16_t echo[PACKET_BODY_LENGTH] = {0x0, 0xbbbb};
uint16_t stat[PACKET_BODY_LENGTH] = {0x1, 0xaaaa};
uint16_t idle_[PACKET_BODY_LENGTH] = {0x2, 0xcccc};
uint16_t shutdown_[PACKET_BODY_LENGTH] = {0x3, 0xdddd};
uint16_t enterImu[PACKET_BODY_LENGTH] = {0x4, 0xcccc};
uint16_t getImuData[PACKET_BODY_LENGTH] = {0x5, 0xeeee};
uint16_t enterPointTrack[PACKET_BODY_LENGTH] = {0x7, 0xffff};

void rando();
void loop();

int main()
{
    wiringPiSetup () ;
    wiringPiSPISetup(CHANNEL, 6250000);
    pinMode(5, INPUT);
    pinMode(CHIPSELECT, OUTPUT);
    digitalWrite(CHIPSELECT, HIGH);
    cout << "Starting" << endl;
    while (1) {
       loop();
    }
}

void transmitH(uint16_t *buf, bool verbos);

void transmit(uint16_t *buf) {
  if (transmitMany) {
    transmitMany = false;
    for (int i = 0; i < 30000; i++) {
      transmitH(buf, false);
    }
    printf("Done\n");
  } else {
    transmitH(buf, true);
  }
}

void setBuf(uint16_t* to_send, int& j, uint16_t val) {
    unsigned char * bytes = (unsigned char *) to_send;
    bytes[2 * j] = val >> 8;
    bytes[2 * j + 1] = val % (1 << 8);
    j++;
}

void transmitH(uint16_t *buf, bool verbos) {
  digitalWrite(CHIPSELECT, HIGH);
  digitalWrite(CHIPSELECT, LOW);
  delayMicroseconds(20);
  int j = 0;
  uint16_t to_send[10000];
  setBuf(to_send, j, 0x1234);
  uint16_t checksum = 0;
  for (int i = 0; i < PACKET_BODY_LENGTH; i++) {
      setBuf(to_send, j, buf[i]);

    checksum += buf[i];
  }
  setBuf(to_send, j, checksum);

  setBuf(to_send, j, 0x4321);

  int numIters = 300;

  memset(to_send + j, 0xff, 2 * numIters);
  wiringPiSPIDataRW(CHANNEL, (unsigned char *) to_send, 2 * (numIters));

  int i;
  for (i = 0; i < numIters; i++) {
      if (verbos) {
          cout << "Received: " << to_send[i] << endl;
      }
  }

  for (i = 0; i < numIters; i++) {
      if (to_send[i] == 0x1234) {
          break;
      }
  }
  if (i == numIters) {
      cout << "No response" << endl;
  }
  uint16_t len = to_send[i+1];
  uint16_t responseNumber = to_send[i+2];
  if (verbos) {
      printf("Response header %d ", responseNumber);
      if (responseNumber == RESPONSE_OK) {
          printf("Ok");
      } else if (responseNumber == RESPONSE_BAD_PACKET) {
          printf("Bad packet");
      } else if (responseNumber == RESPONSE_MIRROR_DATA) {
          printf("Mirror data");
      } else if (responseNumber == RESPONSE_ADCS_REQUEST) {
          printf("ADCS request");
      } else {
          printf("Bad header");
      }
      cout << endl;
      printf("---------------------------------\n");
  }
  delayMicroseconds(20);
  digitalWrite(CHIPSELECT, HIGH);
}

void transmitCrappy(uint16_t *buf) {
    /*
  digitalWrite(CHIPSELECT, HIGH);
  digitalWrite(CHIPSELECT, LOW);
  delay(1);
  send16(0x1233, true);
  uint16_t checksum = 0;
  for (int i = 0; i < PACKET_BODY_LENGTH; i++) {
    send16(buf[i], true);
    checksum += buf[i];
  }
  send16(checksum, true);
  send16(0x4321, true);
  for (int i = 0; i < 30; i++) {
    send16(0xffff, true);
  }
  Serial.println("---------------------------------");
  delay(1);
  digitalWrite(CHIPSELECT, HIGH);*/
}

void rando() {
  printf("begin random\n");
  std::atomic<bool> interrupted;
  interrupted.store(false);

  // create a new thread that does stuff in the background
  int i = 0;
  while(true) {
    int nextCommand = rand() % 3;
    if (nextCommand == 0) {
      transmitH(echo, false);
    } else if (nextCommand == 1) {
      transmitH(stat, false);
    } else if (nextCommand == 2) {
      transmitH(idle_, false);
    } else {
      return;
    }
    i++;
  }
  printf("end random, send %d packets\n", i);
}

void loop() {
  // send data only when you receive data:
  char incomingByte;
  cin >> incomingByte;
  if (incomingByte == '\n') {
  } else if (incomingByte == '1') {
    transmit(echo);
  } else if (incomingByte == '2') {
    transmit(stat);
  } else if (incomingByte == '3') {
    transmit(idle_);
  } else if (incomingByte == '4') {
    transmit(enterImu);
  } else if (incomingByte == '5') {
    transmit(getImuData);
  } else if (incomingByte == '6') {
    transmit(enterPointTrack);
  } else if (incomingByte == '7') {
    transmitCrappy(echo);
  } else if (incomingByte == 'r') {
    rando();
  } else if (incomingByte == 'm') {
    transmitMany = true;
  }
}
