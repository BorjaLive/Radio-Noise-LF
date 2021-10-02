#ifndef RN_LF_SERCOM
#define RN_LF_SERCOM

#include <Arduino.h>
#include <stdint.h>

/*
 *  RECEPCION (6, 3 de datos)
 *  0 - 0xF0 
 *  1 - PWM I
 *  2 - PWM D
 *  3 - FLAGS
 *    6 - DIR I
 *    7 - DIR D
 *  4 - CRC = B1+B2+B3 / 3
 *  5 - 0x0F
 *  
 *  ENVIO (7, 4 de datos)
 *  0 - 0xF0
 *  1 - FLAGS
 *    ...
 *    7 - SENSOR 0
 *  ...
 *  4 - FLAGS
 *    0 - SENSOR 31
 *    ...
 *  5 - CRC = B1+B2+B3+B4 / 4
 *  6 - 0x0F
 */

class RNLFSerCom{
private:
  uint8_t dataIn[3], dataOut[7];
  int inPos;
  bool newData;
public:
  RNLFSerCom();
  void init();
  void act();
  bool inline isNewData(){return this->newData;}
  void getData(uint8_t &pwmI, bool &dirI, uint8_t &pwmD, bool &dirD);
  void sendData(uint32_t sensors);
};

/*
void setBit(uint32_t &bits, int pos, bool value){
  if(value)
    bits |= 1UL << pos;
  else
    bits &= ~(1UL << pos);
}

bool getBit(const uint32_t &bits, int pos){
  return (bits & ( 1 << pos )) >> pos;
}
*/

#endif
