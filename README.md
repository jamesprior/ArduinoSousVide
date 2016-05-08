# Arduino Sous Vide Controller

This project is for a homemade sous vide controller.  The physical hardware is a junction box with a GFCI outlet, a sparkfun mechanical relay, an arduino, temperature probe, and an LCD shield.  The hardware controls the electicity flow to the outlet according to the desired temperature and the existing temperature.  I use a hamilton beach coffee urn for the heating vessel.

Because it uses a mechanical relay instead of a solid state relay it divides the PID output into increments and determines how long to activate the mechanical relay in two second increments.

### Parts
- Sparkfun COM-00101 Relay SPST-NO Sealed - 30A
- Adafruit LCD Shield Kit w/ 16x2 Character Display - Blue and White
- DS18B20 Temperature Sensor - Waterproof Digital Thermal Probe Sensor DS18B20
- An Arduino
- Hamilton Beach 40515 42-Cup Coffee Urn, Silver

### About that coffee urn
The coffee urn comes with a 'warming' feature.  That's nice but what it means is that if the temperature of the water goes above 160 degrees farenheit or so it will stop heating with the main element and instead use the warming coil.  It does this by using a temperature sensitive resistor attached to the bottom of the urn.  If you use the same hardware you'll need to remove the base (IIRC it has torx screws or star bits) and look for the resistor.  It was round, white, and about the size of three nickles stacked together.  It has a high impedence when cool and a low impedence when warm.  If you trace the circuit on the bottom you'll see that the warming element is only active when it swaps to low impedence.  I soldered a length of heavy guage wire on either end to bypass it instead of removing it.