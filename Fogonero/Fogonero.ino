#include <QTRSensors.h>
#include <EEPROM.h>
#include "rnlfSerCom.h"

#define N_SENSORS 32
#define PWM_I_F 8
#define PWM_I_R 9
#define PWM_D_F 10
#define PWM_D_R 11

QTRSensors qtrA, qtrB;
const uint8_t SENSOR_COUNT_A = 16;
const uint8_t SENSORS_A[] = {A0, 32, A2, 34, A4, 36, A6, 38, A8, 40, A10, 42, A12, 44, A14, 46};
const uint8_t SENSOR_COUNT_B = 16;
const uint8_t SENSORS_B[] = {A1, 33, A3, 35, A5, 37, A7, 39, A9, 41, A11, 43, A13, 45, A15, 47};
uint16_t sensorValuesA[SENSOR_COUNT_A];
uint16_t sensorValuesB[SENSOR_COUNT_B];

RNLFSerCom sercom;

void setup() {
  pinMode(PWM_I_F, OUTPUT);
  pinMode(PWM_I_R, OUTPUT);
  pinMode(PWM_D_F, OUTPUT);
  pinMode(PWM_D_R, OUTPUT);
  digitalWrite(PWM_I_F, LOW);
  digitalWrite(PWM_I_R, LOW);
  digitalWrite(PWM_D_F, LOW);
  digitalWrite(PWM_D_R, LOW);
  sercom.init();

  qtrA.setTypeRC();
  qtrA.setSensorPins(SENSORS_A, SENSOR_COUNT_A);
  qtrA.setEmitterPin(2);
  qtrB.setTypeRC();
  qtrB.setSensorPins(SENSORS_B, SENSOR_COUNT_B);
  qtrB.setEmitterPin(2);

  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  //Cargar la calibracion en EEPROM
  qtrA.calibrate();
  qtrB.calibrate();
  for(int i = 0; i < SENSOR_COUNT_A; i++){
    uint8_t* maxVal = (uint8_t*) &qtrA.calibrationOn.maximum[i];
    uint8_t* minVal = (uint8_t*) &qtrA.calibrationOn.minimum[i];
    maxVal[0] = EEPROM.read(i*4);
    maxVal[1] = EEPROM.read(i*4 + 1);
    minVal[0] = EEPROM.read(i*4 + 2);
    minVal[1] = EEPROM.read(i*4 + 3);
  }
  for(int i = 0; i < SENSOR_COUNT_B; i++){
    uint8_t* maxVal = (uint8_t*) &qtrB.calibrationOn.maximum[i];
    uint8_t* minVal = (uint8_t*) &qtrB.calibrationOn.minimum[i];
    maxVal[0] = EEPROM.read(SENSOR_COUNT_A * 4 + i*4);
    maxVal[1] = EEPROM.read(SENSOR_COUNT_A * 4 + i*4 + 1);
    minVal[0] = EEPROM.read(SENSOR_COUNT_A * 4 + i*4 + 2);
    minVal[1] = EEPROM.read(SENSOR_COUNT_A * 4 + i*4 + 3);
  }
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  //Leer los datos de conduccion y actualizar los drivers
  sercom.act();
  if(sercom.isNewData()){
    uint8_t pwmI, pwmD;
    bool dirI, dirD, calibrar;
    sercom.getData(pwmI, dirI, pwmD, dirD, calibrar);
    if(calibrar){
      digitalWrite(LED_BUILTIN, HIGH);
      qtrA.resetCalibration();
      qtrB.resetCalibration();
      for (uint16_t i = 0; i < 200; i++){
         qtrA.calibrate();
         qtrB.calibrate();
      }
      //Guardar la calibracion en EEPROM
      for(int i = 0; i < SENSOR_COUNT_A; i++){
        uint8_t* maxVal = (uint8_t*) &qtrA.calibrationOn.maximum[i];
        uint8_t* minVal = (uint8_t*) &qtrA.calibrationOn.minimum[i];
        EEPROM.write(i*4, maxVal[0]);
        EEPROM.write(i*4 + 1, maxVal[1]);
        EEPROM.write(i*4 + 2, minVal[0]);
        EEPROM.write(i*4 + 3, minVal[1]);
      }
      for(int i = 0; i < SENSOR_COUNT_B; i++){
        uint8_t* maxVal = (uint8_t*) &qtrB.calibrationOn.maximum[i];
        uint8_t* minVal = (uint8_t*) &qtrB.calibrationOn.minimum[i];
        EEPROM.write(SENSOR_COUNT_A * 4 + i*4, maxVal[0]);
        EEPROM.write(SENSOR_COUNT_A * 4 + i*4 + 1, maxVal[1]);
        EEPROM.write(SENSOR_COUNT_A * 4 + i*4 + 2, minVal[0]);
        EEPROM.write(SENSOR_COUNT_A * 4 + i*4 + 3, minVal[1]);
      }
      digitalWrite(LED_BUILTIN, LOW);
    }else{
      if(dirI){
        analogWrite(PWM_I_F, pwmI);
        analogWrite(PWM_I_R, 0);
      }else{
        analogWrite(PWM_I_F, 0);
        analogWrite(PWM_I_R, pwmI);
      }
      if(dirD){
        analogWrite(PWM_D_F, pwmD);
        analogWrite(PWM_D_R, 0);
      }else{
        analogWrite(PWM_D_F, 0);
        analogWrite(PWM_D_R, pwmD);
      }
    }
  }

  //Enviar los datos de los sensores

  // read calibrated sensor values and obtain a measure of the line position
  // from 0 to 5000 (for a white line, use readLineWhite() instead)
  uint16_t posA = qtrA.readLineBlack(sensorValuesA);
  uint16_t posB = qtrB.readLineBlack(sensorValuesB);

  // print the sensor values as numbers from 0 to 1000, where 0 means maximum
  // reflectance and 1000 means minimum reflectance, followed by the line
  // position#
  sercom.sendData(0, sensorValuesB, sensorValuesA, SENSOR_COUNT_A, SENSOR_COUNT_B);
  
  delay(1000/30);
}
