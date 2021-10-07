#include "rnlfSerCom.h"

RNLFSerCom::RNLFSerCom(){
  this->inPos = 0;
  this->dataOut[0] = 0xF0;
  this->dataOut[37] = 0x0F;
  this->newData = false;
}

void RNLFSerCom::init(){
  Serial.begin(19200);
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

void RNLFSerCom::getData(uint8_t &pwmI, bool &dirI, uint8_t &pwmD, bool &dirD, bool &calibrar){
  pwmI = dataIn[0];
  pwmD = dataIn[1];
  dirI = bitRead(dataIn[2], 0);
  dirD = bitRead(dataIn[2], 1);
  calibrar = bitRead(dataIn[2], 2);
  this->newData = false;
}

void RNLFSerCom::sendData(uint16_t pos, uint16_t *sensorsA, uint16_t *sensorsB, uint8_t nsensorsA, uint8_t nsensorsB){
  dataOut[1] = 0;
  
  uint8_t* posBytes = (uint8_t*) &pos;
  dataOut[2] = posBytes[1];
  dataOut[3] = posBytes[0];
  
  for(int i = 0; i < nsensorsA; i++){
    dataOut[i+4] = map(sensorsA[i], 0, 1000, 255, 0);
  }
  for(int i = 0; i < nsensorsB; i++){
    dataOut[i+4+nsensorsA] = map(sensorsB[i], 0, 1000, 255, 0);
  }
  
  int crc = 0;
  for(int i = 1; i < 36; i++){
    crc += dataOut[i];
  }
  dataOut[36] = crc/36;
  
  Serial.write(dataOut, sizeof(dataOut));
}
