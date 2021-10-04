#include "rnlfSerCom.h"
#define N_SENSORS 8
#define PWM_I_F 3
#define PWM_I_R 9
#define PWM_D_F 10
#define PWM_D_R 11

const int SENSORS[] = {22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52};

RNLFSerCom sercom;
uint32_t sensors;

void setup() {
  for(int i = 0; i < N_SENSORS; i++){
    pinMode(SENSORS[i], INPUT);
  }
  pinMode(PWM_I_F, OUTPUT);
  pinMode(PWM_I_R, OUTPUT);
  pinMode(PWM_D_F, OUTPUT);
  pinMode(PWM_D_R, OUTPUT);
  digitalWrite(PWM_I_F, LOW);
  digitalWrite(PWM_I_R, LOW);
  digitalWrite(PWM_D_F, LOW);
  digitalWrite(PWM_D_R, LOW);
  sercom.init();
}

void loop() {
  //Leer los datos de conduccion y actualizar los drivers
  sercom.act();
  if(sercom.isNewData()){
    uint8_t pwmI, pwmD;
    bool dirI, dirD;
    sercom.getData(pwmI, dirI, pwmD, dirD);
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

  //Enviar los datos de los sensores
  for(int i = 0; i < N_SENSORS; i++){
    bitWrite(sensors, i, digitalRead(SENSORS[i]));
  }
  sercom.sendData(sensors);
  
  delay(1000/30);
}
