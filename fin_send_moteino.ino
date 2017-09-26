#include <RFM69.h>
#include <RFM69_ATC.h>

//*********************************************************************************************
//************ IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE ************
//*********************************************************************************************
#define NODEID        2   // must be unique for each node on same network (range up to 254, 255 is used for broadcast)
#define NETWORKID     69  // the same on all nodes that talk to each other (range up to 255)
#define GATEWAYID     1   // this is the one we're sending to
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY   RF69_433MHZ
//#define FREQUENCY   RF69_868MHZ
#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleencryption" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW_HCW  //uncomment only for RFM69HW/HCW! Leave out if you have RFM69W/CW!
//Auto Transmission Control - dials down transmit power to save battery
//Usually you do not need to always transmit at max output power
//By reducing TX power even a little you save a significant amount of battery power
//This setting enables this gateway to work with remote nodes that have ATC enabled to
//dial their power down to only the required level (ATC_RSSI)
#define ENABLE_ATC    //comment out this line to disable AUTO TRANSMISSION CONTROL
#define ATC_RSSI      -80

#define LED           9 // Moteinos have LEDs on D9

#define SWITCH_PIN   14 // corner pin, near gnd pad
#define MIN_CHANGE  1000  // don't send any more often than this many milliseconds

#define SERIAL_BAUD   115200

static char *         msg = "fin";
static byte           sendSize=0;
static boolean        requestACK = false;
static boolean        last;
static unsigned long  nextchangetime;

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif

static void
sendpkt()
{
  if (radio.sendWithRetry(GATEWAYID, msg, strlen(msg))) {
    Serial.print(" ok!");
  } else {
    Serial.print(" nothing...");
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(100);
    digitalWrite(LED, LOW);
    delay(100);
    digitalWrite(LED, HIGH);
    delay(100);
  }
}

void
setup()
{
  Serial.begin(SERIAL_BAUD);
  radio.initialize(FREQUENCY, NODEID, NETWORKID);
#ifdef IS_RFM69HW_HCW
  radio.setHighPower(); //must include this only for RFM69HW/HCW!
#endif
  radio.encrypt(ENCRYPTKEY);

//Auto Transmission Control - dials down transmit power to save battery (-100 is the noise floor, -90 is still pretty good)
//For indoor nodes that are pretty static and at pretty stable temperatures (like a MotionMote) -90dBm is quite safe
//For more variable nodes that can expect to move or experience larger temp drifts a lower margin like -70 to -80 would probably be better
//Always test your ATC mote in the edge cases in your own environment to ensure ATC will perform as you expect
#ifdef ENABLE_ATC
  radio.enableAutoPower(ATC_RSSI);
#endif

  pinMode(SWITCH_PIN, INPUT_PULLUP);
}

void
loop()
{
  boolean       state;
  unsigned long now;

  state = digitalRead(SWITCH_PIN);

  if (state != last) {
    now = millis();

    if (state && (now > nextchangetime)) {
      digitalWrite(LED, HIGH);
      sendpkt();

      nextchangetime = now + MIN_CHANGE;
      digitalWrite(LED, LOW);
    }

    last = state;
  }
}
