# LOGITECH SHIFTER TO USB - G29, G920 & G923 - RASPBERRY PICO
LOGITECH SHIFTER TO USB VIA PICO
Logitech Shifter to USB — G29, G920 & G923 — Raspberry Pi Pico

Convert a Logitech Driving Force shifter into a standalone USB game controller using a Raspberry Pi Pico.

This project reads the Logitech shifter directly, detects gears 1–6 plus reverse, and presents them to the PC as USB joystick buttons. No Logitech wheel base is required.



Hardware Required -

Raspberry Pi Pico

Logitech Driving Force shifter, commonly used with G29, G920 and G923 wheel bases

DB9 connector or suitable cable/adapter

Hook-up wire

Soldering tools or breadboard/jumper wires for testing



DB9 Shifter Pinout

DB9 pin 4  -> Pico GP26 / ADC0, physical pin 31

DB9 pin 8  -> Pico GP27 / ADC1, physical pin 32

DB9 pin 2  -> Pico GP15, physical pin 20

DB9 pin 6  -> Pico GND

DB9 pin 9  -> Pico 3V3 OUT, physical pin 36 - DO NOT USE 5V!

DB9 pin 3  -> DB9 pin 7



SETUP-

Software - Arduino IDE Setup

This project uses the Earle Philhower RP2040 Arduino core.

Install the board package:

Open Arduino IDE

Go to File → Preferences

Add this to Additional Boards Manager URLs:

https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json



Part 1 — Gear Calibration

Used to read raw X/Y/reverse values from the shifter and confirm the wiring is correct.

How to use:

1. Open Serial Monitor at 115200 baud.

2. Put the shifter into a gear and hold it there.

3. Type 1, 2, 3, 4, 5, 6, or r and press Enter.

4. Hold the gear steady for the 5 second capture.

5. Type p to print the current calibration table.



Part 2 — Neutral Calibration / Gear Detection

Used to test the neutral dead-band and gear-detection logic before enabling USB joystick output.

Enter your measured calibration figures in here.



Part 3 — Final USB Joystick Firmware

The final firmware. This turns the Pico into a USB game controller. This still requires the calibration figures.



FINAL TESTING - 

Windows - Press Start - Search for "run": - Type joy.cpl - Select the Pico controller - Open Properties.

Move through each gear and confirm buttons 1–7 activate correctly.

ENJOY!



TROUBLESHOOTING

X and Y stay around the middle and do not change

Likely causes:

Wrong DB9 pin numbering

DB9 pin 9 power not connected to Pico 3V3 OUT

DB9 pin 6 ground not connected to Pico GND

X/Y wires connected to the wrong Pico pins

Shifter connector viewed from the wrong side while wiring


X and Y work, but reverse does not

Check that DB9 pin 2 is connected to Pico GP15, physical pin 20.

On this tested build, an assembly mistake connected reverse to GP14 instead of GP15, causing reverse to appear stuck.


Reverse activates when pushing the lever down, before selecting reverse

This is expected behaviour from the reverse switch.

The final firmware fixes this by requiring both the reverse switch and the correct X/Y reverse-gate position.


Gear is detected when moving sideways in neutral

Use the neutral dead-band version of the firmware.

The final firmware treats the middle Y range as neutral, so left/centre/right movement alone should not select a gear.


Controller does not appear in Windows

Check:

The Earle Philhower RP2040 core is selected

Board is set to Raspberry Pi Pico from the RP2040 board package

USB Stack is set to Pico SDK

The final sketch includes #include <Joystick.h>

The Pico has been unplugged and reconnected after flashing.



Disclaimer

Use this project at your own risk. Check your wiring carefully before plugging anything in. I am not responsible for damage caused by incorrect wiring, incorrect DB9 pin numbering, or applying 5 V to Pico GPIO/ADC pins.
