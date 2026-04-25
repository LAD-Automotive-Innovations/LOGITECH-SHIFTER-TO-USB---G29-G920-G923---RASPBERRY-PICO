// Logitech shifter USB joystick adapter for Raspberry Pi Pico
// Board/core: "Raspberry Pi Pico/RP2040 by Earle F. Philhower, III"
// Arduino IDE: Tools -> USB Stack should be "Pico SDK" / built-in USB, NOT Adafruit TinyUSB.
//
// Wiring used:
// DB9 pin 4  X axis    -> Pico GP26 / ADC0, physical pin 31
// DB9 pin 8  Y axis    -> Pico GP27 / ADC1, physical pin 32
// DB9 pin 2  Reverse   -> Pico GP15, physical pin 20
// DB9 pin 6  GND       -> Pico GND
// DB9 pin 9  Power     -> Pico 3V3 OUT, physical pin 36
// DB9 pin 3  CS/pullup -> 3V3 OUT, preferably through ~10k resistor if needed
// DB9 pin 7            -> DB9 pin 3 only, if your guide requires that jumper
//
// Output mapping:
// 1st     -> USB joystick button 1
// 2nd     -> USB joystick button 2
// 3rd     -> USB joystick button 3
// 4th     -> USB joystick button 4
// 5th     -> USB joystick button 5
// 6th     -> USB joystick button 6
// Reverse -> USB joystick button 7
// Neutral -> no buttons pressed

#include <Joystick.h>

const int X_PIN = 26;        // GP26 / ADC0
const int Y_PIN = 27;        // GP27 / ADC1
const int REV_PIN = 15;      // GP15 digital input

// Lane thresholds based on your measured calibration.
// X lanes: left = 1/2, centre = 3/4, right = 5/6/R
const int X_LEFT_CENTRE_SPLIT = 1635;
const int X_CENTRE_RIGHT_SPLIT = 2463;

// Y engagement thresholds.
// Anything between these two values is treated as neutral.
const int Y_UPPER_GEAR_MIN = 3000;
const int Y_LOWER_GEAR_MAX = 1200;

// Reverse is electrically triggered by pushing the stick down,
// so do not report R from the switch alone.
// Require reverse switch HIGH and lower-right gate position.
const int R_X_MIN = 2850;

char lastGear = '?';

void setup() {
  pinMode(REV_PIN, INPUT_PULLUP);
  analogReadResolution(12);  // Pico ADC range: 0-4095

  Joystick.begin();
  Joystick.useManualSend(true);

  releaseAllGearButtons();
  centreUnusedAxes();
  Joystick.send_now();
}

void loop() {
  int x = analogRead(X_PIN);
  int y = analogRead(Y_PIN);
  int reverseRaw = digitalRead(REV_PIN);

  char gear = detectGear(x, y, reverseRaw);

  if (gear != lastGear) {
    sendGearAsJoystickButtons(gear);
    lastGear = gear;
  }

  delay(10);
}

char detectGear(int x, int y, int reverseRaw) {
  bool upper = y >= Y_UPPER_GEAR_MIN;
  bool lower = y <= Y_LOWER_GEAR_MAX;

  // Neutral/dead band: sideways movement alone should not select a gear.
  if (!upper && !lower) {
    return 'N';
  }

  bool left = x < X_LEFT_CENTRE_SPLIT;
  bool centre = x >= X_LEFT_CENTRE_SPLIT && x < X_CENTRE_RIGHT_SPLIT;
  bool right = x >= X_CENTRE_RIGHT_SPLIT;

  // Reverse switch is active-high, but it also triggers when the stick is merely depressed.
  // Therefore require the right/lower reverse area as well.
  bool reverseArea = (x >= R_X_MIN && lower);
  if (reverseRaw == HIGH && reverseArea) {
    return 'R';
  }

  if (left && upper) return '1';
  if (left && lower) return '2';
  if (centre && upper) return '3';
  if (centre && lower) return '4';
  if (right && upper) return '5';
  if (right && lower) return '6';

  return 'N';
}

void sendGearAsJoystickButtons(char gear) {
  releaseAllGearButtons();

  // Joystick.setButton() is zero-based:
  // setButton(0) appears as button 1 in most game-controller testers.
  switch (gear) {
    case '1': Joystick.setButton(0, true); break;
    case '2': Joystick.setButton(1, true); break;
    case '3': Joystick.setButton(2, true); break;
    case '4': Joystick.setButton(3, true); break;
    case '5': Joystick.setButton(4, true); break;
    case '6': Joystick.setButton(5, true); break;
    case 'R': Joystick.setButton(6, true); break;
    case 'N':
    default:
      // Neutral: leave all gear buttons released.
      break;
  }

  centreUnusedAxes();
  Joystick.send_now();
}

void releaseAllGearButtons() {
  for (uint8_t i = 0; i < 7; i++) {
    Joystick.setButton(i, false);
  }
}

void centreUnusedAxes() {
  // The RP2040 Joystick library exposes axes by default.
  // Keep them centred/quiet so games only see gear buttons changing.
  Joystick.X(512);
  Joystick.Y(512);
  Joystick.Z(512);
  Joystick.Zrotate(512);
  Joystick.sliderLeft(0);
  Joystick.sliderRight(0);
  Joystick.hat(-1);
}
