#include "rnlfSerCom.h"

RNLFSerCom::RNLFSerCom(){
  this->inPos = 0;
  this->dataOut[0] = 0xF0;
  this->dataOut[6] = 0x0F;
  this->newData = false;
}

void RNLFSerCom::init(){
  Serial.begin(9600);
}

void RNLFSerCom::act(){
  while(Serial.available()){
    uint8_t b = Serial.read();
    
    switch(this->inPos){
      case 0:
        if(b != 0xF0) this->inPos = 0;
        else  this->inPos++;
      break;
      case 4:
        if(b != (dataIn[0]+dataIn[1]+dataIn[2])/3) this->inPos = 0;
        else this->inPos++;
      break;
      case 5:
        if(b == 0x0F) this->newData = true;
        this->inPos = 0;
      break;
      default:
        dataIn[this->inPos-1] = b;
        this->inPos++;
    }
  }
}

void RNLFSerCom::getData(uint8_t &pwmI, bool &dirI, uint8_t &pwmD, bool &dirD){
  pwmI = dataIn[0];
  pwmD = dataIn[1];
  dirI = bitRead(dataIn[2], 0);
  dirD = bitRead(dataIn[2], 1);
  this->newData = false;
}

void RNLFSerCom::sendData(uint32_t sensors){
  uint8_t* bytes = (uint8_t*) &sensors;
  int crc = 0;
  for(int i = 0; i < 4; i++){
    dataOut[i+1] = bytes[3-i];
    crc += bytes[3-i];
  }
  dataOut[5] = crc/4;
  Serial.write(dataOut, sizeof(dataOut));
}
