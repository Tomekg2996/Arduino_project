#include <SPI.h>
#include <Servo.h>
#include "nRF24L01.h"
#include "RF24.h"

#define PWM_FORWARD 5
#define PWM_BACKWARD 6

//Remember that this code is the same as in the transmitter
const uint64_t pipeIn = 0xE8E8F0F0E1LL; 

//Create Servo object
Servo control_servo;
//Set up nRF24L01 radio on SPI bus plus pin 9, 10
RF24 radio(9, 10);

//definition of variable 
int direct, motor_speed, control_servo_value;

/**************************************************/

 //We define each byte of data input, in this case just 6 channels
struct Data {
  byte AxisX;
  byte AxisY;
};

Data data;

/**************************************************/

void resetData(){
//We define the inicial value of each data input
//3 potenciometers will be in the middle position so 127 is the middle from 254
  data.AxisX = 127;
  data.AxisY = 127;
}

/**************************************************/

void setup(){
  Serial.begin(9600); 	//Set the speed to 9600 bauds if you want.
  			//You should always have the same speed selected in the serial monitor
  //Setup and configutation RF radio
  resetData();
  radio.begin();
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1,pipeIn);
  
  //start the radio comunication
  radio.startListening();
  
  //attaches the servo on pin A0
  control_servo.attach(A0);
  
  //set pins as output
  pinMode(PWM_FORWARD, OUTPUT);
  pinMode(PWM_BACKWARD, OUTPUT);
}

/**************************************************/

unsigned long lastRecvTime = 0;

//function reading receive data
void recvData(){
  while ( radio.available() ) {
    radio.read(&data, sizeof(Data));
    //here we receive the data
    lastRecvTime = millis();
  }
}

/**************************************************/

void loop(){
  recvData();
  unsigned long now = millis();
  //Here we check if we've lost signal, if we did we reset the values 
  if (now - lastRecvTime > 1000) {
    // Signal lost?
    resetData();
  }

  Serial.print("Axis X: "); Serial.print(data.AxisX);  Serial.print("    ");
  Serial.print("Axis Y: "); Serial.print(data.AxisY);  Serial.println("    ");

  control_servo_value = abs((data.AxisY / 1.42) - 180);
  control_servo.write(control_servo_value);
  motor_speed = (data.AxisX) - 126;

  if(motor_speed > 15){
    forward(motor_speed);
  }else if(motor_speed < (-15)){
    motor_speed = abs(motor_speed);
    backward(motor_speed);
  }else{
    stop_motor();
  }
}

/**************************************************/

void forward(int speed){
  analogWrite(PWM_FORWARD, speed);
  analogWrite(PWM_BACKWARD, 0);
}

void backward(int speed){
  analogWrite(PWM_BACKWARD, speed);
  analogWrite(PWM_FORWARD, 0);
}

void stop_motor(){
  analogWrite(PWM_BACKWARD,0);
  analogWrite(PWM_FORWARD,0);
}

