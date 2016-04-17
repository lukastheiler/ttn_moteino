# The Thing Network - Arduino Monteino Lora walkthrough
Step by step guide to get the Monteino Lora edition up and running.
Shopping list:

- [Monetino](https://lowpowerlab.com/shop/Moteino/moteinomega), the RFM95 LoRa868 version. They have an [USB](https://lowpowerlab.com/shop/Moteino/MoteinoMEGAUSB) version which probably is easier to get started, else you'd  need an USB-TTL converter.

# Sodering

### D0 and D1 pins

Ouf ot the box, the Monteino is meant for peer to peer communication. In order to make the SX1276 chip talk to the things network, you'll have to soder at least the D0 and D1 pin. My sodering skills are far beyond [Urs Marti](https://github.com/urs8000)'s so I just show you his image, D0/D1 are the red cables, the yellow one is reset (not really needed).
<img src="https://github.com/lukastheiler/ttn_monteino/blob/master/images/monteinomega_for_lora-ttn_1024.jpg">

### Antenna

You'll also need to attach an antenna. I just took a copper wire of 8.2cm length, and attach it to the antenna.
<img src="https://github.com/lukastheiler/ttn_monteino/blob/master/images/antenna.jpg" height="50%" width="50%">

# Arduino code
I started out with this repository https://github.com/matthijskooijman/arduino-lmic which made the IBM LIC library availbably to arduino. The arduino code is in this git.

## Changes

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
 The [ttn fair access policy](http://forum.thethingsnetwork.org/t/limitations-data-rate-packet-size-30-seconds-day-fair-access-policy-nodes-per-gateway/1300) allows 30s/day and node. Which translates to roughly 1 message every 3 mins. If you send too many, the gateway stops accepting messages. If you don't get any messages anymore, consider switching the node address, or restart your gateway with ```sudo systemctl start ttn-gateway.service```.

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
