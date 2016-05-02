# The Thing Network - Arduino Moteino Lora Walkthrough

Update: with the new backend this is work in progress! I'm trying to figure the whole thing out, this is what works so far.  

## RN2482 and otaa Walkthrough


## Hardware and Software
I'm using this Board from [Microchip](http://www.microchip.com/DevelopmentTools/ProductDetails.aspx?PartNO=dm164138#utm_medium=Press-Release&utm_term=LoRa%20Certification%20&utm_content=WPD&utm_campaign=868MHz) and [CoolTerm](http://freeware.the-meiers.org) to connect to it with with 9600 Baud.

## Preparation steps
Basically, follow the guide from http://staging.thethingsnetwork.org/wiki/Backend/ttnctl/QuickStart. Download ttnctl, sign up, create an application.

## First steps RN2482 and otaa
On the RN2482, with ```sys get hweui``` and ```mac get deveui``` you get the devices hweui & deveui (they are probably the same). For this example i've set up a new deveui:

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
             {0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x00, 0x00, 0x5A}

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
  mac tx uncnf 1 48656c6C6F
  > ok
  > mac_tx_ok
  ```
  Watch the mqtt channel:
  ```
  INFO Subscribed. Waiting for messages...
  INFO Hello                                    DevEUI=FEEDFEEDFEEDFEED
  WARN Sending data as plain text is bad practice. We recommend to transmit data in a binary format. DevEUI=FEEDFEEDFEEDFEED
  ```

Success!! ... to be continued :)




## Gateway
Update it often, sometimes, these commands are your friend:
```
ttn@ttn-gateway:~ $ sudo tail -f /var/log/daemon.log                # See what's going on
ttn@ttn-gateway:~ $ sudo systemctl restart ttn-gateway.service      # Restart the gateway
ttn@ttn-gateway:~ $ cd ic880a-gateway && sudo ./install.sh spi      # Update the gateway
```



# Previous guide (outdated with the new backend)
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
I started out with this repository https://github.com/matthijskooijman/arduino-lmic which made the IBM LIC library availbably to arduino. The arduino code is in this git repository.

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
```static const u4_t DEVADDR = 0xFEEDBEEF```, the addresses are non-inique.
Check the [address space] (http://staging.thethingsnetwork.org/wiki/Legacy/AddressSpace)

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

## On Things Newtork
Go check your result using the thingsnetwork restful api: http://thethingsnetwork.org/api/v0/nodes/FEEDBEEF/

## Using mqtt
Check out the mqtt script in this respository, and you get all messages in mqtt, e.g:
```
➜  ttn_monteino git:(master) ✗ node mqtt.js
connected, subscribed nodes/FEEDBEEF/packets
[1] Sun Apr 17 2016 20:13:13 GMT+0200 (CEST) from nodes/FEEDBEEF/packets
{ gatewayEui: 'B827EBFFFEC7F595',
  nodeEui: 'FEEDBEEF',
  time: '2016-04-17T18:13:13.505977494Z',
  frequency: 867.1,
  dataRate: 'SF7BW125',
  rssi: -41,
  snr: 10.8,
  rawData: 'QO3+SFqACwAButTT0jE9kHKoTrV+ObIHEQe2Zhc=',
  data: 'Arduino count=11' }
```
