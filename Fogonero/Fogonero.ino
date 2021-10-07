#include <QTRSensors.h>
#include "rnlfSerCom.h"

#define N_SENSORS 16
#define PWM_I_F 3
#define PWM_I_R 9
#define PWM_D_F 10
#define PWM_D_R 11

#define QTR_DIM_A_ODD A0
#define QTR_DIM_A_EVEN A1
#define QTR_DIM_B_ODD A2
#define QTR_DIM_B_EVEN A3

QTRSensors qtrA, qtrB;
const uint8_t SENSOR_COUNT_A = 16;
const uint8_t SENSORS_A[] = {38, 22, 40, 24, 42, 26, 44, 28, 46, 30, 48, 32, 50, 34, 52, 36};
const uint8_t SENSOR_COUNT_B = 16;
const uint8_t SENSORS_B[] = {39, 23, 41, 25, 43, 26, 45, 29, 47, 31, 49, 33, 51, 35, 53, 37};
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

  // 2.5 ms RC read timeout (default) * 10 reads per calibrate() call
  // = ~25 ms per calibrate() call.
  // Call calibrate() 400 times to make calibration take about 10 seconds.
  for (uint16_t i = 0; i < 40; i++){
    qtrA.calibrate();
    qtrB.calibrate();
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
  sercom.sendData(posA, sensorValuesA, sensorValuesB, SENSOR_COUNT_A, SENSOR_COUNT_B);
  
  delay(1000/30);
}
