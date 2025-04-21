#include <AFMotor.h>     // подключаем библиотеку для шилда
#include "Servo.h" //Подключение библиотеки управления сервоприводом

int trigPin = A0; 
int echoPin = A1;


long duration;
int distance; //переменная для вычисления расстояния
Servo servo; //создаем объект типа Servo

AF_DCMotor motorFrontLeft(3);  //  передний левый
AF_DCMotor motorFrontRight(4); //  передний правый
AF_DCMotor motorRearLeft(2);   //  задний левый
AF_DCMotor motorRearRight(1);  //  задний правый

byte SPEED_CURRENT = 0; // текущая скорость моторов

const byte MOTOR_ROTATE_RIGHT = 0;  // вправо резкий разворот на месте (все левые колеса крутятся вперед, все правые - назад)
const byte MOTOR_TURN_RIGHT   = 1;  // вправо плавный поворот
const byte MOTOR_ROTATE_LEFT  = 2;  // влево резкий разворот на месте
const byte MOTOR_TURN_LEFT    = 3;  // влево плавный поворот
const byte MOTOR_TURN_BACK_RIGHT = 4; // поворот вправо задним ходом
const byte MOTOR_TURN_BACK_LEFT  = 5;

byte MOTOR_PREV_DIRECTION; // предыдущее выполненное направление движения
byte RADAR_PREV_DIRECTION = 0; // предыдущее выполненное направление движения радара

const int DELAY_RUN    = 2;
const int DELAY_RUN_BACK = 50;
const int DELAY_ROTATE = 300;
const int DELAY_TURN   = 500;
const int DELAY_TURN_BACK = 500;

const int DST_TRH_TURN = 30;

const int DST_TRH_BACK = 15;

byte debug = 2;
void setup() {
servo.attach(10);
//устанавливает режим работы портов на ардуино
  // - выход
  pinMode(trigPin, OUTPUT); 
  // - вход
  pinMode(echoPin, INPUT);
  
    motorInit();
    
  if (debug  > 1) {
    delay(3000);
    
  }
  
  radar_default();
  
  Serial.begin(9600); // подключаем монитор порта
}

void loop() {
  
  if (debug  > 1) Serial.println("\n*** new loop() start ***\n");
  // сравнить измеренные расстояния до препятствий
  // и определить направления движения
  int distance;
  // замер расстояния
  distance = (measureDistance() + measureDistance() + measureDistance()) /3 ;
//  distance = measureDistance();
  if (debug > 1) {
    Serial.print("distance = "); Serial.println(distance);
  }
  // препятствие так близко что надо ехать назад ?
  if ( distance <= DST_TRH_BACK ) {
    if (debug > 1) Serial.println("ALARM! Distance too small!!!");
    // стоп
    motorStop();
    // отъезжаем назад
    motorRunBack();
    // проверяем куда поворачивали ранее
    if (debug  > 1) delay(1000);
    // ранее уже поворачивали задним ходом влево?
    if (MOTOR_TURN_BACK_LEFT == MOTOR_PREV_DIRECTION) {
      motorRotateRight();
    } else {
      motorRotateLeft();
    }
    return;
  }

 
  if ( distance > DST_TRH_TURN )   {
    motorRunForward();
  } else {
    motorStop();
    radar();
  }

    if ( distance < 45 ) {
        if (MOTOR_TURN_BACK_LEFT == MOTOR_PREV_DIRECTION) {
      motorTurnRight();
    } else {
      motorTurnLeft();
    }
  }
}


void motorInit()  {
  if (debug > 1) Serial.println("motor Init");
  if (debug > 5) return;
  // turn on motor
  motorSetSpeed(190); // скорость мотора 0--255, реально меньше 100 не работает
  motorStop();
}

// движение вперед по прямой
void motorRunForward()  {
  if (debug > 1) Serial.println("Forward");
  if (debug > 5) return;
  motorFrontLeft.run(FORWARD);
  motorFrontRight.run(FORWARD);
  motorRearLeft.run(FORWARD);
  motorRearRight.run(FORWARD);
  delay(DELAY_RUN);
}

// движение назад по прямой
void motorRunBack()  {
  if (debug > 1) Serial.println("Backward");
  if (debug > 5) return;
  motorFrontLeft.run(BACKWARD);
  motorFrontRight.run(BACKWARD);
  motorRearLeft.run(BACKWARD);
  motorRearRight.run(BACKWARD);
  delay(DELAY_RUN_BACK);
}

// правый разворот на месте
void motorRotateRight()  {
  MOTOR_PREV_DIRECTION = MOTOR_ROTATE_RIGHT;
  if (debug > 1) Serial.println("Rotate R");
  if (debug > 5) return;
  motorFrontLeft.run(FORWARD);
  motorFrontRight.run(BACKWARD);
  motorRearLeft.run(FORWARD);
  motorRearRight.run(BACKWARD);
  delay(DELAY_ROTATE);
}

// правый плавный поворот (при движении вперед)
void motorTurnRight()  {
  MOTOR_PREV_DIRECTION = MOTOR_TURN_RIGHT;
  if (debug > 1) Serial.println("Turn R");
  if (debug > 5) return;
  motorFrontLeft.run(FORWARD);
  motorFrontRight.run(RELEASE);
  motorRearLeft.run(FORWARD);
  motorRearRight.run(RELEASE);
  delay(DELAY_TURN);
}

// правый плавный поворот (при движении назад)
void motorTurnBackRight()  {
  MOTOR_PREV_DIRECTION = MOTOR_TURN_BACK_RIGHT;
  if (debug > 1) Serial.println("Turn Back R");
  if (debug > 5) return;
  motorFrontLeft.run(BACKWARD);
  motorFrontRight.run(RELEASE);
  motorRearLeft.run(BACKWARD);
  motorRearRight.run(RELEASE);
  delay(DELAY_TURN_BACK);
}

// левый разворот на месте
void motorRotateLeft()  {
  MOTOR_PREV_DIRECTION = MOTOR_ROTATE_LEFT;
  if (debug > 1) Serial.println("Rotate L");
  if (debug > 5) return;
  motorFrontLeft.run(BACKWARD);
  motorFrontRight.run(FORWARD);
  motorRearLeft.run(BACKWARD);
  motorRearRight.run(FORWARD);
  delay(DELAY_ROTATE);
}

// левый плавный поворот (при движении вперед)
void motorTurnLeft()  {
  MOTOR_PREV_DIRECTION = MOTOR_TURN_LEFT;
  if (debug > 1) Serial.println("Turn L");
  if (debug > 5) return;
  motorFrontLeft.run(RELEASE);
  motorFrontRight.run(FORWARD);
  motorRearLeft.run(RELEASE);
  motorRearRight.run(FORWARD);
  delay(DELAY_TURN);
}

// левый плавный поворот (при движении назад)
void motorTurnBackLeft()  {
  MOTOR_PREV_DIRECTION = MOTOR_TURN_BACK_LEFT;
  if (debug > 1) Serial.println("Turn Back L");
  if (debug > 5) return;
  motorFrontLeft.run(RELEASE);
  motorFrontRight.run(BACKWARD);
  motorRearLeft.run(RELEASE);
  motorRearRight.run(BACKWARD);
  delay(DELAY_TURN_BACK);
}

// стоп резко
void motorStop()  {
  if (debug > 1) Serial.println("Stop");
  if (debug > 5) return;
  motorFrontLeft.run(RELEASE);
  motorFrontRight.run(RELEASE);
  motorRearLeft.run(RELEASE);
  motorRearRight.run(RELEASE);
}

// стоп плавно
void motorStopSlow()  {
  if (debug > 1) Serial.println("Stop slow");
  if (debug > 5) return;
  int speed;
  int diff = SPEED_CURRENT / 3; // сбрасываем скорость в 3 приема
  for (speed = SPEED_CURRENT; speed <= 0; speed -= diff) {
    motorSetSpeed(speed);
    delay(150);
  }
  motorStop(); // тормозим еще раз на всякий случай
}

// разгон плавно
void motorRunSlow()  {
  if (debug) Serial.println("Stop slow");
  if (debug > 5) return;
  int speed;
  int diff = (255 - SPEED_CURRENT) / 3; // набираем скорость в 3 приема
  for (speed = SPEED_CURRENT; speed > 255; speed += diff) {
    motorSetSpeed(speed);
    delay(150);
  }
  motorSetSpeed(255); // устанавливаем максималку еще раз на всякий случай
}

// установить скорость 0--255
void motorSetSpeed(int speed)  {
  // скорость мотора 0--255
  if (speed > 255)
    speed = 255;
  if (speed < 0)
    speed = 0;
  if (debug) {
    Serial.print("Motor set Speed = ");
    Serial.println(speed);
  }
  if (debug > 5) return;
  motorFrontLeft.setSpeed(speed);
  motorFrontRight.setSpeed(speed);
  motorRearLeft.setSpeed(speed);
  motorRearRight.setSpeed(speed);
  // запоминаем текущую скорость
  SPEED_CURRENT = speed;
}

int radar()
{
  if (RADAR_PREV_DIRECTION == 1) {
  for (int pos = 70; pos <= 130; pos += 1)
  {
    servo.write(pos);
    Serial.print(" ");
    distance = measureDistance();
    Serial.println(distance);
    delay(25);
    if (distance > DST_TRH_TURN) {
          if (MOTOR_TURN_BACK_RIGHT == MOTOR_PREV_DIRECTION) {
            motorRotateLeft();
            } else {
              motorRotateRight();
              }
      radar_default();
      RADAR_PREV_DIRECTION = 0;
      return RADAR_PREV_DIRECTION;
    }
  }
  } else {

    for (int pos = 190; pos >= 130; pos -= 1)
    {
    servo.write(pos);
    Serial.print(" ");
    distance = measureDistance();
    Serial.println(distance);
    delay(25);
      if (distance > DST_TRH_TURN) {
            if (MOTOR_TURN_BACK_LEFT == MOTOR_PREV_DIRECTION) {
              motorRotateRight();
              } else {
                motorRotateLeft();
                }
        radar_default();
        RADAR_PREV_DIRECTION = 1;
        return RADAR_PREV_DIRECTION;
    }
  }
  }
  
}


int radar_default() {
    servo.write(130);
    Serial.print(" ");
    distance = measureDistance();
    Serial.println(distance);

}

int measureDistance()
{
  long duration, cm;
   // для большей точности установим значение LOW на пине Trig
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Теперь установим высокий уровень на пине Trig
  digitalWrite(trigPin, HIGH);
  // Ждем 10 милисекунд
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
   // Узнаем длительность высокого сигнала на пине Echo
  duration = pulseIn(echoPin, HIGH);
  //вычисляем расстояние до объекта
  cm = duration / 58; 
  return cm;
}
