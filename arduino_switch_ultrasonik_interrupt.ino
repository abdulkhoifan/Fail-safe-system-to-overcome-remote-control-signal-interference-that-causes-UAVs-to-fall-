#define PWM_MIN 994
#define PWM_MID 1490
#define PWM_MAX 1988
#include <SPI.h>  
#include <Pixy.h>
#include <Servo.h>
#include <ESC.h> 
#include <EnableInterrupt.h>

#define RC_NUM_CHANNELS  2
#define RC_CH3  0
#define RC_CH7  1
#define RC_CH3_INPUT  3
#define RC_CH7_INPUT  5
#define THR_OUT       6

Servo myESC;
int pinTrigger = 13;
int pinEcho = 12;
float waktu;
float jarak;
float throttle;
uint16_t rc_values[RC_NUM_CHANNELS];
uint32_t rc_start[RC_NUM_CHANNELS];
volatile uint16_t rc_shared[RC_NUM_CHANNELS];

void rc_read_values() 
{
  noInterrupts();
  memcpy(rc_values, (const void *)rc_shared, sizeof(rc_shared));
  interrupts();
}

void calc_input(uint8_t channel, uint8_t input_pin) {
  if (digitalRead(input_pin) == HIGH) {
    rc_start[channel] = micros();
  } else {
    uint16_t rc_compare = (uint16_t)(micros() - rc_start[channel]);
    rc_shared[channel] = rc_compare;
  }
}

void calc_ch3() { calc_input(RC_CH3, RC_CH3_INPUT); }
void calc_ch7() { calc_input(RC_CH7, RC_CH7_INPUT); }

void setup() 
{
   pinMode(pinTrigger, OUTPUT); //pin trigger sebagai output
   pinMode(pinEcho, INPUT); //

  pinMode(RC_CH3_INPUT, INPUT);
  pinMode(RC_CH7_INPUT, INPUT);
  
  pinMode (THR_OUT, OUTPUT);

  enableInterrupt(RC_CH3_INPUT, calc_ch3, CHANGE);
  enableInterrupt(RC_CH7_INPUT, calc_ch7, CHANGE);

  Serial.begin(57600); //spesific baudrate fir wireless com

}
///////////////////////////////////////////////////////////////////////////////////////////////////////////

// fungsi dibawah ini untuk mengeluarkan output PWM dari arduino ke RC flight controller
void Throttle()
{ 
  throttle=(rc_values[RC_CH3]-480.73)/10.852;
Serial.print(" PWM Throttle = ");
Serial.println(throttle);
delay(10);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void BacaJarak()
{
    digitalWrite(pinTrigger, LOW);
    delayMicroseconds(2);
    digitalWrite(pinTrigger, HIGH);
    delayMicroseconds(10);
    digitalWrite(pinTrigger, LOW);
    waktu = pulseIn(pinEcho, HIGH);
    jarak = (waktu * 0.034) / 2;
    Serial.print("jarak ");
    Serial.print(jarak);
    Serial.println(" cm");
    delay(50); 
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void loop() 
{
rc_read_values();
Serial.print("CH3:"); Serial.print(rc_values[RC_CH3]); Serial.print("\t");
Serial.print("CH7:"); Serial.println(rc_values[RC_CH7]);

  Throttle();
  BacaJarak();

  if (rc_values[RC_CH7]>=1500) // kondisi jika aux switch ch7 diaktifkan
  
 {    
    
      if(jarak <=100)
      {
        myESC.attach(6);
        myESC.write(93);
      }
      
      else 
      {
        myESC.attach(6);
        myESC.write(throttle);
      } 
 }
 
 else 
    {
      myESC.attach(6); //output ke PPM ch3 (trhottle)
      myESC.write(throttle);
    }
 
}
