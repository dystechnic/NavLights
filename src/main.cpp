#include <EnableInterrupt.h>

#define SERIAL_PORT_SPEED 9600

#define RC_NUM_CHANNELS  1
#define RC_CH1  0
// #define RC_CH2  1

// pins receiver is connected to
#define RC_CH1_INPUT  14  //  == pin A0
// #define RC_CH2_INPUT  15  //  == pin A1

// definition of signal thresholds
#define INPUT_LOW_THRESHOLD 1000 // Low to Mid input signal threshold. 
#define INPUT_MID_THRESHOLD 1400 // Mid to High input signal threshold.
#define INPUT_HIGH_THRESHOLD 1800 // Mid to High input signal threshold.

//  Landing light settings
#define LL_PIN_LIGHT 9 // Landing light output pin number

// Anti-collision beacon settings
#define ACB1_PIN_LIGHT 6 // Pin number for anti-collision beacon 1
#define ACB2_PIN_LIGHT 5 // Pin number for anti-collision beacon 2
#define ACB_FADE_MIN 10 // Minimum fade level for anti-collision beacon (0-255)
#define ACB_FADE_MAX 75 // Maximum fade level for anti-collision beacon (0-255)
#define ACB_FADE_INTERVAL 25000 // Fade step interval, in microseconds (lower numbers = faster fade)

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

void calc_ch1() { 
  calc_input(RC_CH1, RC_CH1_INPUT); 
}

void debug() {
  Serial.print("CH1:"); Serial.print(rc_values[RC_CH1]); Serial.print("\t");
}

// Turn on or off landing light
void setLandingLight(boolean state)
{
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
  if (rc_values[RC_CH1] > INPUT_HIGH_THRESHOLD) {
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
  //  Add as many as you need
  pinMode(RC_CH1_INPUT, INPUT);
  //  pinMode(RC_CH2_INPUT, INPUT);

  //  Enable interrupts on input channels
  enableInterrupt(RC_CH1_INPUT, calc_ch1, CHANGE);
  //  enableInterrupt(RC_CH2_INPUT, calc_ch2, CHANGE);
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
   // Check if it's time to fade the anti-collision lights
    if ((currentTime - lastFadeTime) > ACB_FADE_INTERVAL) {
      doFade();
      lastFadeTime = currentTime;
    }    checkLandingLight();

  // Check if it's time to blink the strobes
    if ((currentTime - lastStrobeTime) > STB_BLINK_INTERVAL) {
      doStrobe();
      lastStrobeTime = currentTime; 
    }

  }
  
  debug(); // enable for serial debugging of receiver inputs
}
