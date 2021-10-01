/* 
2 witte leds (landinglights) = schakelbaar en aan bij flaps down
1 groene navigatie led rechts = schakelbaar
1 rode navigatie led links = schakelbaar
1 witte led achter = schakelbaar
2 rode (strobe) beacon lights = schakelbaar, faden aan en uit
 */

#include <EnableInterrupt.h>

#define SERIAL_PORT_SPEED 9600
#define RC_NUM_CHANNELS  2

// inout channels from receiver
#define RC_CH1  0
#define RC_CH2  1
// #define RC_CH3  2
// #define RC_CH4  3

#define RC_CH1_INPUT  A0
#define RC_CH2_INPUT  A1
//  #define RC_CH3_INPUT  A2
//  #define RC_CH4_INPUT  A3

#define INPUT_LOW_THRESHOLD 1000 // Low to Mid input signal threshold. 
#define INPUT_MID_THRESHOLD 1500 // Mid to High input signal threshold.

//  Landing light settings
//  #define LL_IRQ_NUMBER 0 // Interrupt number to use (0 = pin 2 on most boards)
//  #define LL_PIN_SERVO 2 // Servo input pin number - this needs to match whatever interrupt is used
#define LL_PIN_LIGHT 9 // Landing light output pin number
//  #define LL_SERVO_THRESHOLD 1500 // Servo signal threshold to turn on/off landing light (pulse width in microseconds, 1000 to 2000)
//  #define LL_SERVO_DEAD_ZONE 100 // Servo signal dead-zone size, eliminates flicker
//  #define LL_SERVO_REVERSED true   // Whether or not the servo channel is reversed

// Anti-collision beacon settings
#define ACB1_PIN_LIGHT 6 // Pin number for anti-collision beacon 1
#define ACB2_PIN_LIGHT 5 // Pin number for anti-collision beacon 2
#define ACB_FADE_MIN 10 // Minimum fade level for beacon (0-255)
#define ACB_FADE_MAX 75 // Maximum fade level for beacon (0-255)
#define ACB_FADE_INTERVAL 30000 // Fade step interval, in microseconds (lower numbers = faster fade)

// Strobe settings
#define STB_PIN_LIGHT 3 // Pin number for strobe light output
#define STB_BLINK_INTERVAL 2000000 // Blink interval for strobe light in microseconds

// Navigation lights settings
#define NAV_PIN_LIGHT 10 // Pin number for strobe light output

// Var declarations
boolean curLandingLight = false;
boolean curLightSwitch = false;

uint16_t rc_values[RC_NUM_CHANNELS];
uint32_t rc_start[RC_NUM_CHANNELS];
volatile uint16_t rc_shared[RC_NUM_CHANNELS];

unsigned long lastFadeTime = 0;
unsigned long lastStrobeTime = 0;
int currentFade = ACB_FADE_MIN;
int fadeDirection = 1;

void rc_read_values() {
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

void calc_ch1() { calc_input(RC_CH1, RC_CH1_INPUT); }
void calc_ch2() { calc_input(RC_CH2, RC_CH2_INPUT); }
// void calc_ch3() { calc_input(RC_CH3, RC_CH3_INPUT); }
// void calc_ch4() { calc_input(RC_CH4, RC_CH4_INPUT); }

void debug() {
  Serial.print("CH1:"); Serial.print(rc_values[RC_CH1]); Serial.print("\t");
  Serial.print("CH2:"); Serial.println(rc_values[RC_CH2]);
//  Serial.print("CH2:"); Serial.print(rc_values[RC_CH2]); Serial.println("\t");
//  Serial.print("CH3:"); Serial.print(rc_values[RC_CH3]); Serial.print("\t");
//  Serial.print("CH4:"); Serial.println(rc_values[RC_CH4]);
//  delay(200);
}

// Turn on or off landing light
void setLandingLight(boolean state)
{
//  float i;
  if (state && !curLandingLight) {
    digitalWrite(LL_PIN_LIGHT, HIGH);
  } else if (!state && curLandingLight) {
    digitalWrite(LL_PIN_LIGHT, LOW);
  }
  curLandingLight = state;
}

void checkLandingLight()
{
  // Check LL input position
  if (rc_values[RC_CH2] >= INPUT_MID_THRESHOLD) {
    setLandingLight(true);
  } else {
    setLandingLight(false);
  }
}

void allLightsOff(){
  digitalWrite(NAV_PIN_LIGHT, LOW);
  digitalWrite(STB_PIN_LIGHT, LOW);
  digitalWrite(ACB1_PIN_LIGHT, LOW);
  digitalWrite(ACB2_PIN_LIGHT, LOW);
  //  digitalWrite(LL_PIN_LIGHT, LOW);
  setLandingLight(false);
}


// Fade anti-collision LEDs
void doFade()
{
  currentFade += fadeDirection;
  if (currentFade == ACB_FADE_MAX || currentFade == ACB_FADE_MIN) {
    // If we hit the fade limit, flash the high beacon, and flip the fade direction
    if (fadeDirection == 1) {
      analogWrite(ACB1_PIN_LIGHT, 255);

    } else {
      analogWrite(ACB2_PIN_LIGHT, 255);
    }
    delay(50); 
    fadeDirection *= -1; 
  }

  analogWrite(ACB1_PIN_LIGHT, currentFade);
  analogWrite(ACB2_PIN_LIGHT, ACB_FADE_MAX - currentFade + ACB_FADE_MIN);
}

void doStrobe()
{
  digitalWrite(STB_PIN_LIGHT, HIGH);
  delay(50);
  digitalWrite(STB_PIN_LIGHT, LOW);
  delay(50);
  digitalWrite(STB_PIN_LIGHT, HIGH);
  delay(50);
  digitalWrite(STB_PIN_LIGHT, LOW);
}

void setLightSwitch(boolean state)
{
  if (state && !curLightSwitch) {
    digitalWrite(NAV_PIN_LIGHT, HIGH);
  } else if (!state && curLightSwitch) {
    allLightsOff();
  }
  curLightSwitch = state;
}

void checkLightSwitch()
{
  if (rc_values[RC_CH1] > INPUT_LOW_THRESHOLD) {
    setLightSwitch(true);
  }
  else {
    setLightSwitch(false);
  }
  }

void setup() {
  Serial.begin(SERIAL_PORT_SPEED);
 
  //  Declare output pins
  pinMode(LL_PIN_LIGHT, OUTPUT);
  pinMode(STB_PIN_LIGHT, OUTPUT);
  pinMode(ACB1_PIN_LIGHT, OUTPUT);
  pinMode(ACB2_PIN_LIGHT, OUTPUT);
  pinMode(NAV_PIN_LIGHT, OUTPUT);
 
  //  Declaration of input pins 
  pinMode(RC_CH1_INPUT, INPUT);
  pinMode(RC_CH2_INPUT, INPUT);
  //  pinMode(RC_CH3_INPUT, INPUT);
  //  pinMode(RC_CH4_INPUT, INPUT);

  //  Enable interrupts on input channels
  enableInterrupt(RC_CH1_INPUT, calc_ch1, CHANGE);
  enableInterrupt(RC_CH2_INPUT, calc_ch2, CHANGE);
//  enableInterrupt(RC_CH3_INPUT, calc_ch3, CHANGE);
//  enableInterrupt(RC_CH4_INPUT, calc_ch4, CHANGE);
}

void loop() {
  unsigned long currentTime = micros();
  rc_read_values();
  // Check if lightswitch is on or off
  checkLightSwitch();
  if (!curLightSwitch) {
    // nothing to do. Light switch is off.
  }
  else {
    checkLandingLight();
   // Check if it's time to fade the anti-collision lights
    if ((currentTime - lastFadeTime) > ACB_FADE_INTERVAL) {
      doFade();
      lastFadeTime = currentTime;
    }

  // Check if it's time to blink the strobes
    if ((currentTime - lastStrobeTime) > STB_BLINK_INTERVAL) {
      doStrobe();
      lastStrobeTime = currentTime; 
    }

  }
  
  //  debug(); // enable for serial debugging of receiver inputs
}
