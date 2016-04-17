# The Thing network - Arduino Monteino Lora walkthrough
Step by step guide to get the Monteino Lora edition up and running.
Shopping list:

- [Monetino](https://lowpowerlab.com/shop/Moteino/moteinomega), the RFM95 LoRa868 version. They have an [USB](https://lowpowerlab.com/shop/Moteino/MoteinoMEGAUSB) version, else you'd  need an USB-TTL converter.

# Sodering
Ouf ot the box, the Monteino is meant for peer to peer communication. In order to make the SX1276 chip talk to the things network, you'll have to soder at least the D0 and D1 pin. My sodering skills are far beyond Urs Marti's so I just show you his image, D0/D1 are the red cables, the yellow one is reset (not really needed).
![monteinomega_for_lora-ttn_1024.jpg](https://github.com/lukastheiler/ttn_monteino/blob/master/images/monteinomega_for_lora-ttn_1024.jpg)

You'll also need to attach an antenna. I just took a copper wire of 8.2cm length, and attach it to the antenna.
![antenna.jpg](https://github.com/lukastheiler/ttn_monteino/blob/master/images/antenna.jpg)

# Results

http://thethingsnetwork.org/api/v0/nodes/5A48DEAD/
