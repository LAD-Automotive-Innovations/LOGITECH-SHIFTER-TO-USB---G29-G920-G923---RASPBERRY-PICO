// Logitech shifter calibration helper for Raspberry Pi Pico
//
// Wiring used for this test:
// DB9 pin 4  X axis    -> Pico GP26 / ADC0, physical pin 31
// DB9 pin 8  Y axis    -> Pico GP27 / ADC1, physical pin 32
// DB9 pin 2  Reverse   -> Pico GP15, physical pin 20
// DB9 pin 6  GND       -> Pico GND
// DB9 pin 9  Power     -> Pico 3V3 OUT, physical pin 36
// DB9 pin 3  CS/pullup -> 3V3 OUT, preferably through ~10k resistor if needed
// DB9 pin 7            -> DB9 pin 3 only, if your guide requires that jumper
//
// How to use:
// 1. Open Serial Monitor at 115200 baud.
// 2. Put the shifter into a gear and hold it there.
// 3. Type 1, 2, 3, 4, 5, 6, or r and press Enter.
// 4. Hold the gear steady for the 5 second capture.
// 5. Type p to print the current calibration table.
// 6. Type reset to clear all captured values.
//
// Important reverse note:
// The reverse switch appears to trigger when the stick is depressed, before it is fully in the reverse gate.
// Final reverse detection should therefore use BOTH:
//   reverseRaw == HIGH
//   AND X/Y position matching the calibrated reverse location.

const int X_PIN = 26;        // GP26 / ADC0
const int Y_PIN = 27;        // GP27 / ADC1
const int REV_PIN = 15;      // GP15 digital input

const unsigned long SAMPLE_TIME_MS = 5000;
const unsigned long SAMPLE_INTERVAL_MS = 10;

struct GearCalibration {
  char label;
  bool valid;
  int minX;
  int maxX;
  int minY;
  int maxY;
  long sumX;
  long sumY;
  int samples;
  int reverseHighSamples;
};

GearCalibration gears[] = {
  {'1', false, 4095, 0, 4095, 0, 0, 0, 0, 0},
  {'2', false, 4095, 0, 4095, 0, 0, 0, 0, 0},
  {'3', false, 4095, 0, 4095, 0, 0, 0, 0, 0},
  {'4', false, 4095, 0, 4095, 0, 0, 0, 0, 0},
  {'5', false, 4095, 0, 4095, 0, 0, 0, 0, 0},
  {'6', false, 4095, 0, 4095, 0, 0, 0, 0, 0},
  {'R', false, 4095, 0, 4095, 0, 0, 0, 0, 0}
};

const int GEAR_COUNT = sizeof(gears) / sizeof(gears[0]);

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(REV_PIN, INPUT_PULLUP);
  analogReadResolution(12);  // Pico ADC range: 0-4095

  printHelp();
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toUpperCase();

    if (cmd.length() == 0) {
      return;
    }

    if (cmd == "P" || cmd == "PRINT" || cmd == "DONE") {
      printCalibrationTable();
      return;
    }

    if (cmd == "RESET") {
      resetCalibration();
      Serial.println("Calibration data cleared.");
      return;
    }

    if (cmd == "HELP" || cmd == "?") {
      printHelp();
      return;
    }

    char gear = cmd.charAt(0);
    if (gear == 'R' || (gear >= '1' && gear <= '6')) {
      captureGear(gear);
      return;
    }

    Serial.print("Unknown command: ");
    Serial.println(cmd);
    Serial.println("Use 1, 2, 3, 4, 5, 6, r, p, reset, or help.");
  }
}

void captureGear(char gearLabel) {
  int index = findGearIndex(gearLabel);
  if (index < 0) {
    return;
  }

  GearCalibration &g = gears[index];

  g.valid = false;
  g.minX = 4095;
  g.maxX = 0;
  g.minY = 4095;
  g.maxY = 0;
  g.sumX = 0;
  g.sumY = 0;
  g.samples = 0;
  g.reverseHighSamples = 0;

  Serial.println();
  Serial.print("Capturing gear ");
  Serial.print(gearLabel);
  Serial.println(" for 5 seconds. Hold the shifter steady...");

  unsigned long start = millis();
  unsigned long nextProgress = start + 1000;

  while (millis() - start < SAMPLE_TIME_MS) {
    int x = analogRead(X_PIN);
    int y = analogRead(Y_PIN);
    int reverseRaw = digitalRead(REV_PIN);

    if (x < g.minX) g.minX = x;
    if (x > g.maxX) g.maxX = x;
    if (y < g.minY) g.minY = y;
    if (y > g.maxY) g.maxY = y;

    g.sumX += x;
    g.sumY += y;
    g.samples++;

    if (reverseRaw == HIGH) {
      g.reverseHighSamples++;
    }

    if (millis() >= nextProgress) {
      Serial.print(".");
      nextProgress += 1000;
    }

    delay(SAMPLE_INTERVAL_MS);
  }

  g.valid = true;

  Serial.println();
  Serial.print("Captured gear ");
  Serial.println(gearLabel);
  printGear(g);
  Serial.println();
}

int findGearIndex(char label) {
  label = toupper(label);
  for (int i = 0; i < GEAR_COUNT; i++) {
    if (gears[i].label == label) {
      return i;
    }
  }
  return -1;
}

int centreX(const GearCalibration &g) {
  if (g.samples == 0) return 0;
  return g.sumX / g.samples;
}

int centreY(const GearCalibration &g) {
  if (g.samples == 0) return 0;
  return g.sumY / g.samples;
}

int reversePercent(const GearCalibration &g) {
  if (g.samples == 0) return 0;
  return (g.reverseHighSamples * 100L) / g.samples;
}

void printGear(const GearCalibration &g) {
  if (!g.valid) {
    Serial.print(g.label);
    Serial.println(": not captured");
    return;
  }

  Serial.print(g.label);
  Serial.print(": X centre ");
  Serial.print(centreX(g));
  Serial.print("  X min/max ");
  Serial.print(g.minX);
  Serial.print("/");
  Serial.print(g.maxX);

  Serial.print("  | Y centre ");
  Serial.print(centreY(g));
  Serial.print("  Y min/max ");
  Serial.print(g.minY);
  Serial.print("/");
  Serial.print(g.maxY);

  Serial.print("  | reverse HIGH ");
  Serial.print(reversePercent(g));
  Serial.println("%");
}

void printCalibrationTable() {
  Serial.println();
  Serial.println("Calibration table:");
  Serial.println("Gear | X centre | X min/max | Y centre | Y min/max | Reverse HIGH %");
  Serial.println("-----+----------+-----------+----------+-----------+---------------");

  for (int i = 0; i < GEAR_COUNT; i++) {
    printGear(gears[i]);
  }

  Serial.println();
  Serial.println("Suggested final-code approach:");
  Serial.println("- Use X/Y centre positions to identify gears.");
  Serial.println("- Use tolerance around each centre rather than only reverse switch state.");
  Serial.println("- Only report Reverse if reverseRaw == HIGH AND X/Y is near calibrated R.");
  Serial.println();
}

void resetCalibration() {
  for (int i = 0; i < GEAR_COUNT; i++) {
    gears[i].valid = false;
    gears[i].minX = 4095;
    gears[i].maxX = 0;
    gears[i].minY = 4095;
    gears[i].maxY = 0;
    gears[i].sumX = 0;
    gears[i].sumY = 0;
    gears[i].samples = 0;
    gears[i].reverseHighSamples = 0;
  }
}

void printHelp() {
  Serial.println();
  Serial.println("Logitech shifter Pico calibration helper");
  Serial.println("Commands:");
  Serial.println("  1     capture 1st gear for 5 seconds");
  Serial.println("  2     capture 2nd gear for 5 seconds");
  Serial.println("  3     capture 3rd gear for 5 seconds");
  Serial.println("  4     capture 4th gear for 5 seconds");
  Serial.println("  5     capture 5th gear for 5 seconds");
  Serial.println("  6     capture 6th gear for 5 seconds");
  Serial.println("  r     capture reverse for 5 seconds");
  Serial.println("  p     print table");
  Serial.println("  done  print table");
  Serial.println("  reset clear captured values");
  Serial.println();
  Serial.println("Put the shifter in the gear first, then type the command and press Enter.");
  Serial.println("For reverse, fully select reverse gear, not just pushed-down neutral.");
  Serial.println();
}
