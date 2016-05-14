# The Things Network with the Moteino and RN2483

This guide has been modified for the new staging backend, and has the following sections:

1) Moteino, LMIC and ABP Walkthrough

2) Moteino, LMIC and OTAA Walkthrough

3) RN2483 and OTAA Walkthrough

4) Misc, tips, tricks, lessons learned

# 1) Moteino, LMIC and ABP Walkthrough

This step by step guide should get you up and running with the Moteino Lora edition & The Things Network. Shopping list:

- [Moetino](https://lowpowerlab.com/shop/Moteino/moteinomega), the RFM95 LoRa868 version. They have an [USB](https://lowpowerlab.com/shop/Moteino/MoteinoMEGAUSB) version which probably is easier to get started, else you'll  need an USB-TTL converter.

## Sodering

### D0 and D1 pins

Ouf ot the box, the Moteino is meant for peer to peer communication. In order to make the SX1276 chip talk to the things network, you'll have to soder at least the D0 and D1 pin of the chip to the moteino's pins 0 and 1. Note that D2 is already connected with pin 0. My sodering skills are far beyond [Urs Marti](https://github.com/urs8000)'s so I just show you his image, D0/D1 are the red cables, the yellow one is reset (not really needed).
<img src="https://github.com/lukastheiler/ttn_monteino/blob/master/images/monteinomega_for_lora-ttn_1024.jpg">

### Antenna

You'll also need to attach an antenna. I just took a copper wire of 8.2cm length, and attached it to the antenna.
<img src="https://github.com/lukastheiler/ttn_monteino/blob/master/images/antenna.jpg" height="50%" width="50%">

## TTN setup and create a personalized device

Basically, follow the guide from http://staging.thethingsnetwork.org/wiki/Backend/ttnctl/QuickStart. Download ttnctl, sign up, create an application.
Then, use ttnctl to register a personalized device:

  ```
  ➜ ttnctl devices register personalized FEEDBEEF --relax-fcnt
  INFO Generating random NwkSKey and AppSKey...
  INFO Registered personalized device AppSKey=575BA7FACB7922AE444BE50E325E1A5E DevAddr=FEEDBEEF Flags=0 NwkSKey=DCD0F771A7291D4291BFB641C280D5B7
  ```

And check the status

  ```
  ➜ ttnctl devices info FEEDBEEF
  Personalized device:
  DevAddr: FEEDBEEF
           {0xFE, 0xED, 0xBE, 0xEF}
  NwkSKey: DCD0F771A7291D4291BFB641C280D5B7
           {0xDC, 0xD0, 0xF7, 0x71, 0xA7, 0x29, 0x1D, 0x42, 0x91, 0xBF, 0xB6, 0x41, 0xC2, 0x80, 0xD5, 0xB7}
  AppSKey:  575BA7FACB7922AE444BE50E325E1A5E
           {0x57, 0x5B, 0xA7, 0xFA, 0xCB, 0x79, 0x22, 0xAE, 0x44, 0x4B, 0xE5, 0x0E, 0x32, 0x5E, 0x1A, 0x5E}

  FCntDn:  0
  Flags:   -
  ```

The option ```--relax-fcnt```should only be used in testing. If you dont use it, you'll have to register the device again after every reset.
We need these keys to send messages from the Moteino to the Things Network.

## Arduino code
I started out with this repository https://github.com/matthijskooijman/arduino-lmic which made the IBM LIC library availbably to arduino. The arduino code is in this git repository [ttn_moteino_abp](https://github.com/lukastheiler/ttn_moteino/tree/master/ttn_moteino_abp).


### Changes

* **LMIC pin mapping**.
  Adjust them to the corresponding pins.
  ```
  // Pin mapping
  const lmic_pinmap lmic_pins = {
    .nss = 4,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 13,
    .dio = {2, 1, 0},
  };
  ```

* **Node Device address**.
  Insert parameters DevAddr, NwkSKey and AppSKey form an activated device. You get the keys with ```ttnctl devices info FEEDBEEF```:
  ```
  static const u4_t DEVADDR = 0xFEEDBEEF ;
  static const PROGMEM u1_t NWKSKEY[16] = { 0xDC, 0xD0, 0xF7, 0x71, 0xA7, 0x29, 0x1D, 0x42, 0x91, 0xBF, 0xB6, 0x41, 0xC2, 0x80, 0xD5, 0xB7 };
  static const u1_t PROGMEM APPSKEY[16] = { 0x57, 0x5B, 0xA7, 0xFA, 0xCB, 0x79, 0x22, 0xAE, 0x44, 0x4B, 0xE5, 0x0E, 0x32, 0x5E, 0x1A, 0x5E };
  ```

* **The message**.
  I just submit the current counter. Basically, this is where you'd collect and send the sensor data.

  ```
  byte buffer[32];
  int counter = 0;
  void do_send(osjob_t* j) {

    String message = "Count=" + String(counter);
    message.getBytes(buffer, message.length()+1);
    counter++;
    Serial.println("Sending: "+message);
    LMIC_setTxData2(1, (uint8_t*) buffer, message.length() , 0);
  }
  ```

  Important: check the remarks in the [Misc](https://github.com/lukastheiler/ttn_moteino#payload-and-messages) section on resending the same payload.

* **Resend interval**.
 ```const unsigned TX_INTERVAL = 180;```
 The [ttn fair access policy](http://forum.thethingsnetwork.org/t/limitations-data-rate-packet-size-30-seconds-day-fair-access-policy-nodes-per-gateway/1300) allows 30s/day and node. Which translates to roughly 1 message every 3 mins. If you send too many, the gateway stops accepting messages (at least I believe so, when I wrote this there were quite some changes to ttn, which could have influenced my findings). If you don't get any messages anymore, consider switching the node address, or restart your gateway with ```sudo systemctl start ttn-gateway.service```.

## Results

You can use ```ttnctl subscribe``` to see the activity on your application. The messages should pop up like this:

  ```
  ➜ ttnctl subscribe
  INFO Subscribing uplink messages from device ...
  INFO Subscribed. Waiting for messages...
  INFO 436F756E743D34                           DevEUI=00000000AFFE2803
  INFO 436F756E743D35                           DevEUI=00000000AFFE2803
  ```

Or, you take [mosquitto](http://mosquitto.org):

   ```
   ➜ mosquitto_sub -h staging.thethingsnetwork.org -p 1883 -t +/devices/+/up -u USERNAME -P PASSWORD
   ```

The username is the EUI and the password is the Access Key you get from ```ttnctl applications```

Lastly, you can easily write your own mqtt script, an example is in this repository - which also decrypts the payload.

  ```
  ➜ node mqtt.js
  [52] Fri May 06 2016 11:36:43 GMT+0200 (CEST) from +/devices/00000000AFFE2803/up
  { payload: 'Q291bnQ9MjQ=',
    port: 1,
    counter: 24,
    dev_eui: '00000000AFFE2803',
    metadata:
     [ { frequency: 868.1,
         datarate: 'SF7BW125',
         codingrate: '4/5',
         gateway_timestamp: 1051678379,
         gateway_time: '2016-05-06T09:36:42.566536411Z',
         channel: 0,
         server_time: '2016-05-06T09:36:42.474438509Z',
         rssi: -37,
         lsnr: 10.5,
         rfchain: 1,
         crc: 1,
         modulation: 'LORA',
         gateway_eui: 'B827EBFFFEC7F595',
         altitude: 599,
         longitude: 8.45709,
         latitude: 47.37367 } ],
    payload_decrypted: 'Count=24' }
  ```

# 2) Moteino, LMIC and OTAA Walkthrough

I've finally figured that one out thanks to the guys on the ttn forum http://forum.thethingsnetwork.org/t/over-the-air-activation-otaa-with-lmic/1921/11. My current source code is in this repository under [ttn_moteino_otaa](https://github.com/lukastheiler/ttn_moteino/tree/master/ttn_moteino_otaa).

## Preparation steps
Same as above, follow the guide from http://staging.thethingsnetwork.org/wiki/Backend/ttnctl/QuickStart. Download ttnctl, sign up, create an application.

## Hardware and Software
[Moetino](https://lowpowerlab.com/shop/Moteino/moteinomega) (see the sodering bits below) and Thomas Telkamp and Matthijs Kooijman's [port of the LMIC library](https://github.com/matthijskooijman/arduino-lmic).

## TTN setup
Register a new device with ttnctl.

  ```
  ➜ ttnctl devices register DEEDDEEDDEEDDEED
    INFO Generating random AppKey...
    INFO Registered device AppKey=94F00F5C07C2F536438600A54CAFF740 DevEUI=DEEDDEEDDEEDDEED
  ➜ ttnctl devices info DEEDDEEDDEEDDEED
    Dynamic device:
    AppEUI:  YOUR-OWN-APP-EUI
             {0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__}
    DevEUI:  DEEDDEEDDEEDDEED
             {0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED}
    AppKey:  94F00F5C07C2F536438600A54CAFF740
             {0x94, 0xF0, 0x0F, 0x5C, 0x07, 0xC2, 0xF5, 0x36, 0x43, 0x86, 0x00, 0xA5, 0x4C, 0xAF, 0xF7, 0x40}
    Not yet activated
  ➜  ~
  ```

## Arduino code

You can check out the modified code is in this repository, [ttn_moteino_otaa](https://github.com/lukastheiler/ttn_moteino/tree/master/ttn_moteino_otaa).
To get there, I started out with the LMIC's [default ttn script](https://github.com/matthijskooijman/arduino-lmic/blob/master/examples/ttn/ttn.ino), replace everything from the includes to Hello World with:

  ```
  static const u1_t APPEUI[8]  = {0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__}; // IMPORTANT REVERSE BYTES!
  static const u1_t DEVEUI[8]  = {0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE}; // IMPORTANT REVERSE BYTES!
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
  ```

You need to replace the order of the the bytes for APPEUI and DEVEUI. I hate pointer arithmetics, so I just use this node oneliner:

  ```
  ➜  node
  > '0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED'.split(', ').reverse().join(', ')
    '0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE'
  ```

Then, comment out the ```LMIC_setSession``` calls, under LMIC init.  

If you run the script, be very, very patient - it takes anywhere from 1-15 minutes to get the payload sent.

  ```
  Starting
  202: EV_JOINING
  758145: EV_JOINED
  Packet queued
  2078749: EV_TXCOMPLETE (includes waiting for RX windows)
  ```

Eventually, you can check if the device is activated:

  ```
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
  FCntDn:  2
  ```

You can see the messages while running ```ttnctl subscribe DEEDDEEDDEEDDEED``` or the above mqtt script. Note that you can get an EV_REJOIN_FAILED event, in this case I just re-initialize, and to try get an EV_JOINED event again. THere are more error event-codes you might want to cover. 

# 3) RN2483 and OTAA Walkthrough

It's quite easy to use OTAA with the RN2483.

## Hardware and Software
I'm using this board from [Microchip](http://www.microchip.com/DevelopmentTools/ProductDetails.aspx?PartNO=dm164138#utm_medium=Press-Release&utm_term=LoRa%20Certification%20&utm_content=WPD&utm_campaign=868MHz) and [CoolTerm](http://freeware.the-meiers.org) to connect to it with with 9600 Baud.

## Preparation steps
Basically, follow the guide from http://staging.thethingsnetwork.org/wiki/Backend/ttnctl/QuickStart. Download ttnctl, sign up, create an application.

## First steps RN2483 and OTAA
On the RN2483, with ```sys get hweui``` and ```mac get deveui``` you get the devices hweui & deveui (they are probably the same). For this example I've set up a new deveui:

  ```
  (Serial) mac set deveui FEEDFEEDFEEDFEED
  ```

On the Mac, with the ttnctl tools, I've registered the device with:

  ```
  ➜ ttnctl devices register FEEDFEEDFEEDFEED
    INFO Generating random AppKey...
    INFO Registered device AppKey=43D00092E5403B30BE844EA4611A8975 DevEUI=FEEDFEEDFEEDFEED

  ➜ ttnctl devices info FEEDFEEDFEEDFEED
  Dynamic device:
  AppEUI:  YOUR-OWN-APP-EUI
           {0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__}
  DevEUI:  FEEDFEEDFEEDFEED
           {0xFE, 0xED, 0xFE, 0xED, 0xFE, 0xED, 0xFE, 0xED}
  AppKey:  43D00092E5403B30BE844EA4611A8975
           {0x43, 0xD0, 0x00, 0x92, 0xE5, 0x40, 0x3B, 0x30, 0xBE, 0x84, 0x4E, 0xA4, 0x61, 0x1A, 0x89, 0x75}
  Not yet activated
  ```

Now set the RN2483's params and connect:

  ```
  (Serial) mac set appeui YOUR-OWN-APP-EUI
  > ok
  mac set appkey 43D00092E5403B30BE844EA4611A8975
  > ok
  mac join otaa
  > ok
  > accepted
  ```

Note that now you get a lot more info from ttnctl:

  ```
  ➜ ttnctl devices info FEEDFEEDFEEDFEED
    Dynamic device:
    AppEUI:  YOUR-OWN-APP-EUI
             {0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__, 0x__}
    DevEUI:  FEEDFEEDFEEDFEED
             {0xFE, 0xED, 0xFE, 0xED, 0xFE, 0xED, 0xFE, 0xED}
    AppKey:  43D00092E5403B30BE844EA4611A8975
             {0x43, 0xD0, 0x00, 0x92, 0xE5, 0x40, 0x3B, 0x30, 0xBE, 0x84, 0x4E, 0xA4, 0x61, 0x1A, 0x89, 0x75}
    Activated with the following parameters:
    DevAddr: 1D552308
             {0x1D, 0x55, 0x23, 0x08}
    NwkSKey: 55F60F2A4729CE9663BEDF65046FF1DE
             {0x55, 0xF6, 0x0F, 0x2A, 0x47, 0x29, 0xCE, 0x96, 0x63, 0xBE, 0xDF, 0x65, 0x04, 0x6F, 0xF1, 0xDE}
    AppSKey: C29A290D5B895B947F8D1EB0E9E37687
             {0xC2, 0x9A, 0x29, 0x0D, 0x5B, 0x89, 0x5B, 0x94, 0x7F, 0x8D, 0x1E, 0xB0, 0xE9, 0xE3, 0x76, 0x87}
    FCntUp:  0
    FCntDn:  0
  ```

Next, subscribe to the mqtt channel with:

  ```
  ➜  ttnctl subscribe FEEDFEEDFEEDFEED
     INFO Subscribing uplink messages from device ��������
     INFO Subscribed. Waiting for messages...
  ```

And back with the RN2483, you send off a message:

  ```
  mac tx cnf 1 48656c6C6F
  > ok
  > mac_rx 1
  ```

Watch the mqtt channel:

  ```
  INFO Subscribed. Waiting for messages...
  INFO Hello                                    DevEUI=FEEDFEEDFEEDFEED
  WARN Sending data as plain text is bad practice. We recommend to transmit data in a binary format. DevEUI=FEEDFEEDFEEDFEED
  ```

Success!


# 4) Misc, tips, tricks, lessons learned

## Gateway
Update it often, these commands are your friend:

  ```
  ttn@ttn-gateway:~ $ sudo tail -f /var/log/daemon.log                # See what's going on
  ttn@ttn-gateway:~ $ sudo systemctl restart ttn-gateway.service      # Restart the gateway
  ttn@ttn-gateway:~ $ cd ic880a-gateway && sudo ./install.sh spi      # Update the gateway
  ```

## ttnctl
It gets updated quite frequently and I check it on a daily basis.

## Payload and messages
I've noticed that sending the same payload is ignored sometimes. E.g.:
- Count=0 (sent & received)
- Count=1 (sent & received)
- Reboot Moteino
- Count=0 (sent & NOT received)
- Count=1 (sent & NOT received)
- Count=1 (sent & NOT received)
