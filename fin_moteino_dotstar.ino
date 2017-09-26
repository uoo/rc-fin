#include <RFM69.h>
#include <RFM69_ATC.h>
#include "FastLED.h"

#define FREQUENCY     RF69_915MHZ

#define NODEID  1
#define NETWORKID 69

// 16 character encryption key, must be same on all nodes
                    //          1111111
                    // 1234567890123456
#define ENCRYPTKEY    "sampleencryption"
//#define IS_RFM69HW_HCW  //uncomment only for RFM69HW/HCW! Leave out if you have RFM69W/CW!
#define ENABLE_ATC  // Automatic Transmission Control: dial down power to save battery

// How many leds in your strip?
#define NUM_LEDS 33
#define MINDELAY   2
#define MAXDELAY  10
#define MAXLONGDELAY 4
#define LONG 25
#define MAXPAUSE 600
//#define PAUSEFRAC  5
#define COLORFRAC 20
#define NCYCLES   36

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
//#define DATA_PIN 11 // Moteino has SPI MOSI on D11
//#define CLOCK_PIN 13 // Moteino has SPI SCK on D13
// hardware SPI fails on data pin assertion, we'll do software SPI on I2C pins instead
#define CLOCK_PIN 19 // Moteino I2C clock is on D19
#define DATA_PIN 18 // Moteino I2C data is on D18

#define LED 9 // Moteino has LED on pin 9

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif

// Define the array of leds
static CRGB leds[NUM_LEDS];

static int cycle;
static int lastled;
static int led;
static int ledstart;
static int ledend;
static int dtime;
static boolean pause;
static CRGB ledcolor;
static bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

static void
pick()
{
  int maxdelay;

  if (cycle == 0) {
    return;
  }
  
  --cycle;

#ifdef PAUSEFRAC
  pause    = random(PAUSEFRAC) == 0;
#else
  pause = false;
#endif

  maxdelay = (abs(ledend - ledstart) > LONG) ? MAXLONGDELAY : MAXDELAY;
  ledstart = random(NUM_LEDS);
  ledend   = random(NUM_LEDS);
  dtime    = random(pause ? MAXPAUSE : maxdelay) + MINDELAY;
  led      = ledstart;

  if (random(COLORFRAC) == 0) {
    ledcolor = CHSV(random(0x100), 255, 255);
  } else {
    ledcolor = CRGB::Red;
  }
}

static void
dolights()
{
  leds[lastled] = CRGB::Black;

  if (pause) {
    FastLED.show();
    delay(dtime);
    pick();
    return;
  }

  leds[led] = ledcolor;
  FastLED.show();

  lastled = led;

  if (ledstart < ledend) {
    ++led;
  } else {
    --led;
  }

  if (led == ledend) {
    pick();
  }

  delay(dtime);
}

void
setup()
{
  pinMode(LED, OUTPUT);     

  delay(100);
   
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
#ifdef IS_RFM69HW_HCW
  radio.setHighPower(); //must include this only for RFM69HW/HCW!
#endif
  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(promiscuousMode);

  FastLED.addLeds<DOTSTAR, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS);
  randomSeed(analogRead(A0) + analogRead(A1));
}

void
loop()
{
  if (radio.receiveDone()) {
    // got a message, start display cycle
    cycle = NCYCLES;
    pick();
    digitalWrite(LED, HIGH);
  }

  if (cycle == 0) {
    // done with display, turn out all LEDs
    for (led = 0; led < NUM_LEDS; ++led) {
      leds[led] = CRGB::Black;
    }
    FastLED.show();
    // turn off active LED
    digitalWrite(LED, LOW);
  } else {
    dolights();
  }
}

