/*******************************************************************************
Original script is Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman

1) ttnctl devices register DEEDDEEDDEEDDEED
 INFO Generating random AppKey...
 INFO Registered device AppKey=94F00F5C07C2F536438600A54CAFF740 DevEUI=DEEDDEEDDEEDDEED

2) ➜  ~ ttnctl devices info DEEDDEEDDEEDDEED
 Dynamic device:
 AppEUI:  YOUR-OWN-APP-EUI
          {0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__}
 DevEUI:  DEEDDEEDDEEDDEED
          {0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED}
 AppKey:  94F00F5C07C2F536438600A54CAFF740
          {0x94, 0xF0, 0x0F, 0x5C, 0x07, 0xC2, 0xF5, 0x36, 0x43, 0x86, 0x00, 0xA5, 0x4C, 0xAF, 0xF7, 0x40}

 Not yet activated


 3) Run this code & be very patient. You should get the following messages:
 Starting
 155: EV_JOINING
 Packet queued
 9276632: EV_JOINED
 Packet queued

 4) You can now see that the device is acivated, and if you run ttnctrl subscribe DEEDDEEDDEEDDEED up, you'll see that the message was sent.
 ➜ ttnctl devices info DEEDDEEDDEEDDEED
 Dynamic device:
 AppEUI:  YOUR-OWN-APP-EUI
          {0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__}
 DevEUI:  DEEDDEEDDEEDDEED
          {0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED}
 AppKey:  94F00F5C07C2F536438600A54CAFF740
          {0x94, 0xF0, 0x0F, 0x5C, 0x07, 0xC2, 0xF5, 0x36, 0x43, 0x86, 0x00, 0xA5, 0x4C, 0xAF, 0xF7, 0x40}
 Activated with the following parameters:
 DevAddr: 1C5055EB
          {0x1C, 0x50, 0x55, 0xEB}
 NwkSKey: 0F0D2D38AA050BFBFE5F42C591568963
          {0x0F, 0x0D, 0x2D, 0x38, 0xAA, 0x05, 0x0B, 0xFB, 0xFE, 0x5F, 0x42, 0xC5, 0x91, 0x56, 0x89, 0x63}
 AppSKey: 59E7EEAA7463948FC844A48DE70DD707
          {0x59, 0xE7, 0xEE, 0xAA, 0x74, 0x63, 0x94, 0x8F, 0xC8, 0x44, 0xA4, 0x8D, 0xE7, 0x0D, 0xD7, 0x07}
 FCntUp:  1
 FCntDn:  0


 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

static const u1_t APPEUI[8]  = {0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__}; // REVERSE ORDER
static const u1_t DEVEUI[8]  = {0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE}; // REVERSE ORDER
static const u1_t APPKEY[16] = {0x94, 0xF0, 0x0F, 0x5C, 0x07, 0xC2, 0xF5, 0x36, 0x43, 0x86, 0x00, 0xA5, 0x4C, 0xAF, 0xF7, 0x40};

// provide APPEUI (8 bytes, LSBF)
void os_getArtEui (u1_t* buf) {
  memcpy(buf, APPEUI, 8);
}

// provide DEVEUI (8 bytes, LSBF)
void os_getDevEui (u1_t* buf) {
  memcpy(buf, DEVEUI, 8);
}

// provide APPKEY key (16 bytes)
void os_getDevKey (u1_t* buf) {
  memcpy(buf, APPKEY, 16);
}

static uint8_t mydata[] = "Hello, TTN over OTAA!";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 20;

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
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING: 
      Serial.println(F("EV_JOINING"));
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_RFU1:
      Serial.println(F("EV_RFU1"));
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
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
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    default:
      Serial.println(F("Unknown event"));
      break;
  }
}

void do_send(osjob_t* j) {
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    // Prepare upstream data transmission at the next possible time.
    LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 1);
    Serial.println(F("Packet queued"));
  }
}

// initial job
static void initfunc (osjob_t* j) {
    // reset MAC state
    LMIC_reset();
    // start joining
    LMIC_startJoining();
    // init done - onEvent() callback will be invoked...
}

void setup() {
  osjob_t initjob;
  Serial.begin(115200);
  Serial.println(F("Starting"));
  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  os_setCallback(&initjob, initfunc);

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

  os_runloop();
  // Start job
  do_send(&sendjob);
}

void loop() {
  os_runloop_once();
}
