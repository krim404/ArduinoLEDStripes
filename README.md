# ArduinoLEDStripes
Dual Arduino based LED Stripe Controller. 

Requirement:
* 1 Arduino (ex. UNO)
* 1 Ethernet Shield
* 1 Arduino (ex. Nano)
* 2 RF24 Transceiver
* 3 BUZ11 MOSFET >55V|20A (for > 5Meter LED)
* 1 12V 2A DC Power Supply
* 6 10kOhm Resistor
* 3 390 Ohm Resistor
* Cheap Chinese 12V RGB-LED Stripes
* Optional: LTV847 Optocoupler


Compatible with any mqtt compatible server (ex. Debian / openHAB)

Requires RF24 based wireless transceiver for data exchange.
RF24 Pin => Arduino Pin
GND => GND
3V3 => 3V3
CE => D7
CSN => D8
SCK => D13
MOSI => D11
MISO => D12

Soldering for the Driver:
D3 => 390Ohm Resistor => LTV874 => MOSFET + 10kOhm on Mass => GREEN LED PIN
D5 => 390Ohm Resistor => LTV874 => MOSFET + 10kOhm on Mass => RED LED PIN
D6 => 390Ohm Resistor => LTV874 => MOSFET + 10kOhm on Mass => BLUE LED PIN

3x Mass => MOSFET
3x Mass => 10kOhm Resistor => LTV874

