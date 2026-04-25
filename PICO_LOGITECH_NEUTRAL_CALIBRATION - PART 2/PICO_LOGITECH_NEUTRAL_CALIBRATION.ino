// Logitech shifter gear-detection test for Raspberry Pi Pico
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
// This version does NOT output USB joystick yet.
// It tests gear recognition using your measured calibration values.

const int X_PIN = 26;        // GP26 / ADC0
const int Y_PIN = 27;        // GP27 / ADC1
const int REV_PIN = 15;      // GP15 digital input

// Your measured calibration centres:
const int X_1 = 1203;
const int Y_1 = 3651;
const int X_2 = 1248;
const int Y_2 = 523;
const int X_3 = 2023;
const int Y_3 = 3641;
const int X_4 = 2033;
const int Y_4 = 523;
const int X_5 = 2894;
const int Y_5 = 3693;
const int X_6 = 2926;
const int Y_6 = 548;
const int X_R = 2986;
const int Y_R = 550;

// Lane thresholds based on the calibration centres.
// X lanes: left = 1/2, centre = 3/4, right = 5/6/R
const int X_LEFT_CENTRE_SPLIT = 1635;
const int X_CENTRE_RIGHT_SPLIT = 2463;

// Y engagement thresholds.
// Do NOT use a single halfway split for Y, because left/right neutral can otherwise look like a gear.
// Calibrated upper gears are around 3600-3700.
// Calibrated lower gears are around 520-550.
// Anything between these two thresholds is treated as neutral.
const int Y_UPPER_GEAR_MIN = 3000;
const int Y_LOWER_GEAR_MAX = 1200;

// Reverse is electrically triggered by pushing the stick down,
// so do not report R from the switch alone.
// Require reverse switch HIGH and X/Y close to the reverse gate.
const int R_X_MIN = 2850;
const int R_Y_MAX = 1100;

char lastGear = '?';
unsigned long lastPrint = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(REV_PIN, INPUT_PULLUP);
  analogReadResolution(12);  // Pico ADC range: 0-4095

  Serial.println();
  Serial.println("Logitech shifter gear-detection test");
  Serial.println("Move through 1-6 and reverse. Check detected gear is correct.");
  Serial.println("Neutral band is enabled: sideways movement alone should stay N.");
  Serial.println("Reverse requires reverseRaw HIGH plus X/Y in the reverse area.");
  Serial.println();
}

void loop() {
  int x = analogRead(X_PIN);
  int y = analogRead(Y_PIN);
  int reverseRaw = digitalRead(REV_PIN);

  char gear = detectGear(x, y, reverseRaw);

  if (gear != lastGear || millis() - lastPrint > 500) {
    Serial.print("X: ");
    Serial.print(x);
    Serial.print("   Y: ");
    Serial.print(y);
    Serial.print("   Reverse raw: ");
    Serial.print(reverseRaw);
    Serial.print("   Detected gear: ");
    Serial.println(gear);

    lastGear = gear;
    lastPrint = millis();
  }

  delay(20);
}

char detectGear(int x, int y, int reverseRaw) {
  bool upper = y >= Y_UPPER_GEAR_MIN;
  bool lower = y <= Y_LOWER_GEAR_MAX;

  // Neutral/dead band: the lever may be left, centre, or right while still not engaged.
  // Only report a gear once Y is clearly in the upper or lower gear area.
  if (!upper && !lower) {
    return 'N';
  }

  bool left = x < X_LEFT_CENTRE_SPLIT;
  bool centre = x >= X_LEFT_CENTRE_SPLIT && x < X_CENTRE_RIGHT_SPLIT;
  bool right = x >= X_CENTRE_RIGHT_SPLIT;

  // Reverse switch is active-high, but it also triggers when the stick is merely depressed.
  // Therefore require the right/lower reverse area as well.
  bool reverseArea = (x >= R_X_MIN && y <= Y_LOWER_GEAR_MAX);
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
