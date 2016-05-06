/*******************************************************************************
 Original script is Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman

 ACTIVATION SCRIPT

 1) ttnctl devices register FEEDFEEDFEEDFEED
  INFO Generating random AppKey...
  INFO Registered device AppKey=43D00092E5403B30BE844EA4611A8975 DevEUI=FEEDFEEDFEEDFEED

 2) ➜  ~ ttnctl devices info FEEDFEEDFEEDFEED
  Dynamic device:
  AppEUI:  YOUR-OWN-APP-EUI
           {0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__}

  DevEUI:  FEEDFEEDFEEDFEED
           {0xFE, 0xED, 0xFE, 0xED, 0xFE, 0xED, 0xFE, 0xED}

  AppKey:  43D00092E5403B30BE844EA4611A8975
           {0x43, 0xD0, 0x00, 0x92, 0xE5, 0x40, 0x3B, 0x30, 0xBE, 0x84, 0x4E, 0xA4, 0x61, 0x1A, 0x89, 0x75}

  Not yet activated


  3) Run this code
  --> 161: EV_JOINING

  4) Serial is stuck at "EV_JOINING" but the device activated

  ➜  ~ ttnctl devices info FEEDFEEDFEEDFEED
  Dynamic device:

  AppEUI:  YOUR-OWN-APP-EUI
           {0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__}

  DevEUI:  DEEDDEEDDEEDDEED
           {0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED}

  AppKey:  43D00092E5403B30BE844EA4611A8975
           {0x94, 0xF0, 0x0F, 0x5C, 0x07, 0xC2, 0xF5, 0x36, 0x43, 0x86, 0x00, 0xA5, 0x4C, 0xAF, 0xF7, 0x40}}

  Activated with the following parameters:

  DevAddr: 1D2E0041
           {0x1D, 0x2E, 0x00, 0x41}

  NwkSKey: FFF0B4BB9E765AA15F60115F04A0CCC7
           {0xFF, 0xF0, 0xB4, 0xBB, 0x9E, 0x76, 0x5A, 0xA1, 0x5F, 0x60, 0x11, 0x5F, 0x04, 0xA0, 0xCC, 0xC7}

  AppSKey: AB876E772079FDFDD320A3E3DB1A3D41
           {0xAB, 0x87, 0x6E, 0x77, 0x20, 0x79, 0xFD, 0xFD, 0xD3, 0x20, 0xA3, 0xE3, 0xDB, 0x1A, 0x3D, 0x41}

  FCntUp:  0


 *******************************************************************************/

#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

static const u1_t APPEUI[8]  = {0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__}; // REVERSE ORDER
static const u1_t DEVEUI[8]  = {0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE}; // REVERSE ORDER
static const u1_t APPKEY[16] = {0xAB, 0x87, 0x6E, 0x77, 0x20, 0x79, 0xFD, 0xFD, 0xD3, 0x20, 0xA3, 0xE3, 0xDB, 0x1A, 0x3D, 0x41};

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

static uint8_t mydata[] = "Hello, world!";
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
    osjob_t initjob;

  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    // Prepare upstream data transmission at the next possible time.
    LMIC_setTxData2(1, mydata, sizeof(mydata) - 1, 1);
    Serial.println(F("Packet queued"));
  }
  // Next TX is scheduled after TX_COMPLETE event.
  // os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
  // os_setCallback(&initjob, initfunc);

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
  /*
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

   */
       os_runloop();
  // Start job
  // do_send(&sendjob);
}

void loop() {
  os_runloop_once();
}
