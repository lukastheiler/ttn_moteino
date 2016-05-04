# The Things Network with the Moteino and RN2483

Update: with the new backend, this is work in progress!

## RN2483 and OTAA Walkthrough

I've failed so far connecting the Moteino, sometimes there was a message, sometimes it failed. it took 15 minutes to get messages from the new backend. It's hard to figure out where the source of error is. Therefore took the RN2483 to figure out how the whole thing works.

### Hardware and Software
I'm using this Board from [Microchip](http://www.microchip.com/DevelopmentTools/ProductDetails.aspx?PartNO=dm164138#utm_medium=Press-Release&utm_term=LoRa%20Certification%20&utm_content=WPD&utm_campaign=868MHz) and [CoolTerm](http://freeware.the-meiers.org) to connect to it with with 9600 Baud.

### Preparation steps
Basically, follow the guide from http://staging.thethingsnetwork.org/wiki/Backend/ttnctl/QuickStart. Download ttnctl, sign up, create an application. My first attempts, joining devices via ABP were kind of unstable. So I've switched to OTAA activation.

### First steps RN2483 and OTAA
On the RN2483, with ```sys get hweui``` and ```mac get deveui``` you get the devices hweui & deveui (they are probably the same). For this example i've set up a new deveui:

```
mac set deveui FEEDFEEDFEEDFEED
```

On the Mac, with the ttnctl tools, I've registered the device with:

```
➜ ttnctl devices register FEEDFEEDFEEDFEED
  INFO Generating random AppKey...
  INFO Registered device AppKey=43D00092E5403B30BE844EA4611A8975 DevEUI=FEEDFEEDFEEDFEED

➜ ttnctl devices info FEEDFEEDFEEDFEED
  Dynamic device:

  AppEUI:  YOUR-OWN-APP-EUI
           {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x00, 0x00, 0x5A}

  DevEUI:  FEEDFEEDFEEDFEED
           {0xFE, 0xED, 0xFE, 0xED, 0xFE, 0xED, 0xFE, 0xED}

  AppKey:  43D00092E5403B30BE844EA4611A8975
           {0x43, 0xD0, 0x00, 0x92, 0xE5, 0x40, 0x3B, 0x30, 0xBE, 0x84, 0x4E, 0xA4, 0x61, 0x1A, 0x89, 0x75}

  Not yet activated
```

Now set the RN2483's params and connect:

  ```
  mac set appeui YOUR-OWN-APP-EUI
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

Success!! Moteino is next ... to be continued :)

### Gateway
Update it often, these commands are your friend:
```
ttn@ttn-gateway:~ $ sudo tail -f /var/log/daemon.log                # See what's going on
ttn@ttn-gateway:~ $ sudo systemctl restart ttn-gateway.service      # Restart the gateway
ttn@ttn-gateway:~ $ cd ic880a-gateway && sudo ./install.sh spi      # Update the gateway
```

## Moteino, LMIC and OTAA Walkthrough (WIP)

This is bleeding edge. There's some info on the ttn forum http://forum.thethingsnetwork.org/t/over-the-air-activation-otaa-with-lmic/1921/11 , but I couldn't manage to get messages through yet. My current source code is in this repository under ttn_moteino_new.

### Preparation steps
Same as above, follow the guide from http://staging.thethingsnetwork.org/wiki/Backend/ttnctl/QuickStart. Download ttnctl, sign up, create an application.

### Hardware and Software
[Moetino](https://lowpowerlab.com/shop/Moteino/moteinomega) (see the sodering bits below) and Thomas Telkamp and Matthijs Kooijman's [port of the LMIC library](https://github.com/matthijskooijman/arduino-lmic).

### First steps
You can check out the modified code is in this repository, [ttn_moteino_new](https://github.com/lukastheiler/ttn_moteino/tree/master/ttn_moteino_new). To get there, register a new device with ttnctl.

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

You can start out with the LMIC's [default ttn script](https://github.com/matthijskooijman/arduino-lmic/blob/master/examples/ttn/ttn.ino), replace everything from the includes to Hello World with:

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

You need to replace to reorder the bytes for APPEUI and DEVEUI. I hate pointer arithmetics, so I just use this node oneliner:

```
➜  node
> '0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED'.split(', ').reverse().join(', ')
  '0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE, 0xED, 0xDE'
```

Then, comment out the ```LMIC_setSession``` calls, under LMIC init.

When running the arduino code, I see on the serial monitor:

  ```
  Starting
  Packet queued
  181: EV_JOINING
  ```

And indeed, the activated registered itself:

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

  DevAddr: 1D2E0041
           {0x1D, 0x2E, 0x00, 0x41}

  NwkSKey: FFF0B4BB9E765AA15F60115F04A0CCC7
           {0xFF, 0xF0, 0xB4, 0xBB, 0x9E, 0x76, 0x5A, 0xA1, 0x5F, 0x60, 0x11, 0x5F, 0x04, 0xA0, 0xCC, 0xC7}

  AppSKey: AB876E772079FDFDD320A3E3DB1A3D41
           {0xAB, 0x87, 0x6E, 0x77, 0x20, 0x79, 0xFD, 0xFD, 0xD3, 0x20, 0xA3, 0xE3, 0xDB, 0x1A, 0x3D, 0x41}

  FCntUp:  0
  FCntDn:  0

  ```

Sadly, I don't get any message on the backend. So OTAA is no really working ... to be continued

However, now that the device is activated, you can use these parameters for the next part.

## Arduino Moteino Lora Walkthrough with ABP

Step by step guide to get the Moteino Lora edition up and running.
Shopping list:

- [Moetino](https://lowpowerlab.com/shop/Moteino/moteinomega), the RFM95 LoRa868 version. They have an [USB](https://lowpowerlab.com/shop/Moteino/MoteinoMEGAUSB) version which probably is easier to get started, else you'll  need an USB-TTL converter.

# Sodering

### D0 and D1 pins

Ouf ot the box, the Moteino is meant for peer to peer communication. In order to make the SX1276 chip talk to the things network, you'll have to soder at least the D0 and D1 pin of the chip to the moteino's pins 0 and 1. Note that D2 is already connected with pin 0. My sodering skills are far beyond [Urs Marti](https://github.com/urs8000)'s so I just show you his image, D0/D1 are the red cables, the yellow one is reset (not really needed).
<img src="https://github.com/lukastheiler/ttn_monteino/blob/master/images/monteinomega_for_lora-ttn_1024.jpg">

### Antenna

You'll also need to attach an antenna. I just took a copper wire of 8.2cm length, and attached it to the antenna.
<img src="https://github.com/lukastheiler/ttn_monteino/blob/master/images/antenna.jpg" height="50%" width="50%">

# Arduino code
I started out with this repository https://github.com/matthijskooijman/arduino-lmic which made the IBM LIC library availbably to arduino. The arduino code is in this git repository [ttn_moteino](https://github.com/lukastheiler/ttn_moteino/tree/master/ttn_moteino).

## Changes

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
  Insert parameters DevAddr, NwkSKey and AppSKey form an activated device, e.g. ttnctl devices info DEEDDEEDDEEDDEED from the example above:
  ```
  static const PROGMEM u1_t NWKSKEY[16] = { 0x6A, 0xE2, 0x0C, 0xB2, 0x21, 0x91, 0xF5, 0x15, 0x6F, 0x23, 0xAF, 0xB9, 0x0D, 0x90, 0x5E, 0x62 };
  static const u1_t PROGMEM APPSKEY[16] = { 0x25, 0x0A, 0x37, 0xC2, 0x36, 0xCE, 0x95, 0xE3, 0x0F, 0x8D, 0x60, 0x35, 0x23, 0x98, 0x3C, 0x4E };
  static const u4_t DEVADDR = 0x1C376CC8 ;
  ```

* **The message**.
  I just submit the current counter. Basically, this is where you'd collect and send the sensor data.
  ```
  byte buffer[32];
  int counter = 0;
  void do_send(osjob_t* j) {

    String message = "Arduino count=" + String(counter);
    message.getBytes(buffer, message.length()+1);
    counter++;
    Serial.println("Sending: "+message);
    LMIC_setTxData2(1, (uint8_t*) buffer, message.length() , 0);
  }
  ```

* **Resend interval**.
 ```const unsigned TX_INTERVAL = 180;```
 The [ttn fair access policy](http://forum.thethingsnetwork.org/t/limitations-data-rate-packet-size-30-seconds-day-fair-access-policy-nodes-per-gateway/1300) allows 30s/day and node. Which translates to roughly 1 message every 3 mins. If you send too many, the gateway stops accepting messages (at least I believe so, when I wrote this there were quite some changes to ttn, which could have influenced my findings). If you don't get any messages anymore, consider switching the node address, or restart your gateway with ```sudo systemctl start ttn-gateway.service```.

# Results

## Using mqtt

You can use ```ttnctl subscribe``` to see the activity on your application. The messages should pop up like this:

  ```
  ➜ ttnctl subscribe 70B3D57ED000005A
  INFO Subscribing uplink messages from device p��~�Z
  INFO Subscribed. Waiting for messages...
  INFO 61DE274033F972198377F19BF223D927         DevEUI=DEEDDEEDDEEDDEED
  INFO D0BCF85163B528BC9EB4A57F3C7F6B68         DevEUI=DEEDDEEDDEEDDEED
  ```

Or, you take mosquitto :

   ```
   ➜ mosquitto_sub -h staging.thethingsnetwork.org -p 1883 -t +/devices/+/up -u USERNAME -P PASSWORD
   ```

The username is the EUI and the password is the Access Key you get from ```ttnctl applications```

Lastly, you can easily write your own mqtt script, an example is in this repository - which also decrypts the payload.

  ```
  ➜ node mqtt.js
    [0] Wed May 04 2016 16:57:37 GMT+0200 (CEST) from 70B3D57ED000005A/devices/DEEDDEEDDEEDDEED/up
    { payload: 'QXJkdWlubyBjb3VudD0xMg==',
      port: 1,
      counter: 12,
      dev_eui: 'DEEDDEEDDEEDDEED',
      metadata:
       [ { frequency: 868.3,
           datarate: 'SF7BW125',
           codingrate: '4/5',
           gateway_timestamp: 1316115947,
           gateway_time: '2016-05-04T14:57:36.691644176Z',
           channel: 1,
           server_time: '2016-05-04T14:57:36.617581478Z',
           rssi: -25,
           lsnr: 10.2,
           rfchain: 1,
           crc: 1,
           modulation: 'LORA',
           gateway_eui: 'B827EBFFFEC7F595',
           altitude: 599,
           longitude: 8.45709,
           latitude: 47.37367 } ],
      payload_decrypted: 'Arduino count=12' }
  ```
