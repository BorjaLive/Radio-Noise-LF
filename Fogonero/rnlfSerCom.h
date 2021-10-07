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
 *    5 - CALIBRAR
 *    6 - DIR I
 *    7 - DIR D
 *  4 - CRC = B1+B2+B3 / 3
 *  5 - 0x0F
 *  
 *  ENVIO (38, 36 de datos)
 *  0 - 0xF0
 *  1 - FLAGS
 *    1 - FRONT COLISION DETECTO
 *  2 - POS
 *  3 - POS
 *  4 - SENSOR 0
 *  ...
 *  35 - SENSOR 31
 *  36 - CRC = B1+B2+...+B35 / 35
 *  37 - 0x0F
 */

class RNLFSerCom{
private:
  uint8_t dataIn[3], dataOut[38];
  int inPos;
  bool newData;
public:
  RNLFSerCom();
  void init();
  void act();
  bool inline isNewData(){return this->newData;}
  void getData(uint8_t &pwmI, bool &dirI, uint8_t &pwmD, bool &dirD, bool &calibrar);
  void sendData(uint16_t pos, uint16_t *sensorsA, uint16_t *sensorsB, uint8_t nsensorsA, uint8_t nsensorsB);
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
