// Module Libraries & Dependencies
#include <SPI.h>  // NOTE: temporary, remove after coding

// Switches
const int cycleBase = 2;  // Cycles per Second
const bool  callibrateCSMS  = false;

// Pinouts.
const int CPS_LED_IP  = A3;  // CPS Indicator
const int A_LED_IP    = A4;  // Red Indicator A
const int B_LED_IP    = A5;  // Red Indicator B
const int P_OFF       = A2;  // Power Off Button
const int CSM1      = A0;   // CSM Sensor 1
const int CSM2      = A1;   // CSM Sensor 2
const int CSM3      = A6;   // CSM Sensor 3
const int CSM4      = A7;   // CSM Sensor 4

// Capacitive Soil Moisture Sensor Callibration
// Last date callibrated:      02/15/2024
const int DRY_VAL1  = 619;  // CSMS 1 Dry Value
const int WET_VAL1  = 270;  // CSMS 1 Wet Value

const int DRY_VAL2  = 612;  // CSMS 2 Dry Value
const int WET_VAL2  = 268;  // CSMS 2 Wet Value

const int DRY_VAL3  = 614;  // CSMS 3 Dry Value
const int WET_VAL3  = 267;  // CSMS 3 Wet Value

const int DRY_VAL4  = 617;  // CSMS 4 Dry Value
const int WET_VAL4  = 266;  // CSMS 4 Wet Value

// Cycles per Second Variables
const int ms = 1000;
const int cps = cycleBase * ms;
unsigned long previousTime = millis();


void setup() {
  // Initialize serial monitor.
  Serial.begin(9600);

  // Set pins.
  pinMode(CPS_LED_IP, OUTPUT);
  pinMode(A_LED_IP, OUTPUT);
  pinMode(B_LED_IP, OUTPUT);
  pinMode(P_OFF, INPUT_PULLUP);

  // Initialize modules.


  // Setup peripherals.

}


void loop() {
  // Calculate and check Cycles per Second.
  unsigned long currentTime = millis();
  if (currentTime - previousTime >= cps) {
    previousTime = currentTime;
    cycle();
  }

  // Check power off trigger.
  if (digitalRead(P_OFF) == LOW) {
    powerOffLoop();
  }
}


void powerOffLoop() {
  // Initiate power off procedure.
  digitalWrite(CPS_LED_IP, HIGH);

  // Close files.


  while (true) {
    digitalWrite(A_LED_IP, HIGH);
    digitalWrite(B_LED_IP, HIGH);
    delay(300);
    digitalWrite(A_LED_IP, LOW);
    digitalWrite(B_LED_IP, LOW);
    delay(700);
  }
}


void cycle() {
  // Indicate cycle.
  cycleIndicator();

  // Print Data
  Serial.println(pullCSMData());  // Soil Moisture

  // Print time elapsed.
  // Serial.println(String(millis()/1000));
}


void cycleIndicator() {
  digitalWrite(CPS_LED_IP, HIGH);
  delay(60);
  digitalWrite(CPS_LED_IP, LOW);
  delay(80);
  digitalWrite(CPS_LED_IP, HIGH);
  delay(60);
  digitalWrite(CPS_LED_IP, LOW);
}


String pullCSMData() {
  // Read the analog value from the soil moisture sensor.
  int csm1_raw = analogRead(CSM1);
  int csm2_raw = analogRead(CSM2);
  int csm3_raw = analogRead(CSM3);
  int csm4_raw = analogRead(CSM4);

  // Map the analog value to a percentage. (0-100%)
  int csm1_value = map(csm1_raw, DRY_VAL1, WET_VAL1, 0, 100);
  int csm2_value = map(csm2_raw, DRY_VAL2, WET_VAL2, 0, 100);
  int csm3_value = map(csm3_raw, DRY_VAL3, WET_VAL3, 0, 100);
  int csm4_value = map(csm4_raw, DRY_VAL4, WET_VAL4, 0, 100);

  // List all values.
  int values[] = {
    csm1_value,
    csm2_value,
    csm3_value,
    csm4_value
  };

  String data = "";
  bool invalid = false;
  String err_msg = "";
  // Check data validity (0%-100%) and store them as strings.
  for (int i=0; i < 4; ++i) {
    int mapped_data = values[i];
    if (mapped_data < -5 || mapped_data > 105) {  // 5% Tolerance
      // Renew string data & indicate error.
      data += "None,";
      invalid = true;
      err_msg += String(i+1);
    } else {
      // Concatinate data.
      data += String(mapped_data)+",";
    }
  }

  // Handle error.
  if (invalid) {
    invalidCSMData(err_msg);
  }

  // Calibrate CSM sensor.
  // Output can be fed to a spread sheet
  // to evaluate average value.
  if (callibrateCSMS) {
    String raw_data = "";
    raw_data += "S1 ->\t" + String(csm1_raw);
    raw_data += "\tS2 ->\t" + String(csm2_raw);
    raw_data += "\tS3 ->\t" + String(csm3_raw);
    raw_data += "\tS4 ->\t" + String(csm4_raw);
    return raw_data;
  }

  return data;
}


void invalidCSMData(String n) {
  // Indicator light for invalid CSMS data.
  if (callibrateCSMS) return
  Serial.println("CSMS["+n+"] Error | Value unexpected!");
  digitalWrite(B_LED_IP, HIGH);
  delay(150);
  digitalWrite(B_LED_IP, LOW);
  delay(50);
}
