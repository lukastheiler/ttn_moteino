/*******************************************************************************
  Original script is Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman

  STEPS
  
  1) ➜ ttnctl devices register personalized FEEDBEEF
  INFO Generating random NwkSKey and AppSKey...
  INFO Registered personalized device AppSKey=575BA7FACB7922AE444BE50E325E1A5E DevAddr=FEEDBEEF Flags=0 NwkSKey=DCD0F771A7291D4291BFB641C280D5B7
  ➜  ttn_moteino git:(master) ✗ ttnctl devices info FEEDBEEF
  Personalized device:

  DevAddr: FEEDBEEF
           {0xFE, 0xED, 0xBE, 0xEF}

  NwkSKey: DCD0F771A7291D4291BFB641C280D5B7
           {0xDC, 0xD0, 0xF7, 0x71, 0xA7, 0x29, 0x1D, 0x42, 0x91, 0xBF, 0xB6, 0x41, 0xC2, 0x80, 0xD5, 0xB7}

  AppSKey:  575BA7FACB7922AE444BE50E325E1A5E
           {0x57, 0x5B, 0xA7, 0xFA, 0xCB, 0x79, 0x22, 0xAE, 0x44, 0x4B, 0xE5, 0x0E, 0x32, 0x5E, 0x1A, 0x5E}

  FCntUp:  0
  FCntDn:  0

  Flags:   -

  2) Insert the keys below & run this script

  3) monitor with ttnctl subscribe
  ➜  Downloads ttnctl subscribe
  INFO Subscribing to uplink messages from all devices in application 70B3D57ED000005A
  INFO Subscribed. Waiting for messages...
  INFO 436F756E743D34                           DevEUI=00000000AFFE2803

 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

// LoRaWAN NwkSKey, network session key
// Get these keys from an activated device using:
// ttnctl devices info <DEVICE>
//
static const u4_t DEVADDR = 0xAFFE2803;
static const u1_t PROGMEM NWKSKEY[16] = { 0xCC, 0xDF, 0x44, 0xF1, 0x1E, 0xF5, 0x8E, 0xCE, 0x20, 0xC8, 0x19, 0x0E, 0x12, 0xFD, 0x0B, 0x50 };
static const u1_t PROGMEM APPSKEY[16] = { 0x5F, 0xED, 0x1A, 0xB5, 0x84, 0x86, 0x08, 0x06, 0x29, 0xD2, 0x96, 0x18, 0x96, 0xA3, 0x78, 0x2F };

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static osjob_t sendjob;

// This is the buffer for the data we send

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// Pin mapping
const lmic_pinmap lmic_pins = {
  .nss = 4,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 13,
  .dio = {2, 1, 0},
};

void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch (ev) {
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      if (LMIC.dataLen) {
        // data received in rx slot after tx
        Serial.print(F("Data Received: "));
        Serial.write(LMIC.frame + LMIC.dataBeg, LMIC.dataLen);
        Serial.println();
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    default:
      Serial.println(F("EV_TXCOMPLETE missing"));
      break;
  }
}

byte buffer[32];
int counter = 0;
void do_send(osjob_t* j) {
  String message = "Count=" + String(counter);
  message.getBytes(buffer, message.length()+1);
  counter++;
  Serial.println("Sending: "+message);
  LMIC_setTxData2(1, (uint8_t*) buffer, message.length() , 0);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Starting"));
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  // Set static session parameters. Instead of dynamically establishing a session
  // by joining the network, precomputed session parameters are be provided.
  // On AVR, these values are stored in flash and only copied to RAM
  // once. Copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);

  // Set up the channels used by the Things Network, which corresponds
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set.
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
  // TTN defines an additional channel at 869.525Mhz using SF9 for class B
  // devices' ping slots. LMIC does not have an easy way to define set this
  // frequency and support for class B is spotty and untested, so this
  // frequency is not configured here.

  // Disable link check validation
  LMIC_setLinkCheckMode(0);
  // Set data rate and transmit power (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7, 14);

  // Start job
  do_send(&sendjob);
}

void loop() {
  os_runloop_once();
}
