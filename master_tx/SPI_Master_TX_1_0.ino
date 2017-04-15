#include <SPI.h>

#include <t3spi.h>

//Initialize T3SPI class as SPI_MASTER
T3SPI SPI_MASTER;

//The number of integers per data packet
#define dataLength 256

//Initialize the arrays for outgoing data
//volatile uint8_t  data[dataLength] = {}; 
volatile uint16_t data[dataLength] = {}; 


void setup(){
  
  Serial.begin(115200);
  
  //Begin SPI in MASTER (SCK pin, MOSI pin, MISO pin, CS pin, Active State)
  SPI_MASTER.begin_MASTER(SCK, MOSI, MISO, CS0, CS_ActiveLOW);
  
  //Set the CTAR (CTARn, Frame Size, SPI Mode, Shift Mode, Clock Divider) 
  SPI_MASTER.setCTAR(CTAR_0,16,SPI_MODE0,LSB_FIRST,SPI_CLOCK_DIV4);
  //SPI_MASTER.setCTAR(CTAR_0,16,SPI_MODE0,LSB_FIRST,SPI_CLOCK_DIV2);
  //SPI_MASTER.setCTAR(CTAR_0,16,SPI_MODE0,LSB_FIRST,SPI_CLOCK_DIV128);
  //SPI_MASTER.setCTAR(CTAR_1,8,SPI_MODE0,LSB_FIRST,SPI_CLOCK_DIV128);
  
  //Populate data array 
  for (int i=0; i<dataLength; i++){
    data[i]=i;}

  //Wait for Slave
  delay(5000);
}

void loop(){
  
  //Capture the time before sending data
  SPI_MASTER.timeStamp1 = micros();

  //Send n number of data packets
  for (int i=0; i<1; i++) {
  
    //Send data (data array, data array length, CTARn, CS pin)
    //SPI_MASTER.tx8(data, dataLength,CTAR_1,CS0);}  
    SPI_MASTER.tx16(data, dataLength,CTAR_1,CS0);}
    
  //Capture the time when transfer is done
  SPI_MASTER.timeStamp2 = micros();
  
  //Print data sent & data received
  for (int i=0; i<dataLength; i++){
    Serial.print("data[");
    Serial.print(i);
    Serial.print("]: ");
    Serial.println(data[i]);
    Serial.flush();}

    //Print statistics for the previous transfer
    SPI_MASTER.printStatistics(dataLength);
  
  //Reset the packet count 
  SPI_MASTER.packetCT=0;
  
  delay(2000);
}
