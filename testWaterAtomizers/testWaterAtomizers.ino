// Module Libraries & Dependencies
#include <SPI.h>  // TODO: remove after coding.


// Switches
const int cycleBase = 2;  // Cycles per Second
bool skipAtmozerTest = false;

// Pinouts.
const int CPS_LED_IP  = A3;  // CPS Indicator
const int A_LED_IP    = A4;  // Red Indicator A
const int B_LED_IP    = A5;  // Red Indicator B
const int P_OFF       = A2;  // Power Off Button
const int WA[4] = {2, 3, 4, 5};  // Water Atmomizers 1-4

// Water Atomizers State
bool WA_state[4] = {false, false, false, false};

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


  // Test water atomizers.
  if (!skipAtmozerTest) {
    delay(1500);
    testWA();
  }

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

  // Print time elapsed.
  Serial.println(String(millis()/1000));
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


void testWA () {
  Serial.println("Testing Water Atomizers.");
  int pattern[3] = {1000, 2000, 3000};

  // Loop through and test each atomizer.
  for (int i = 0; i < 4; i++) {
    Serial.println("Testing.. Water Atomizer [" + String(i + 1) + "]");

    for (int n = 0; n < 3; n++) {
      toggleWA(WA[i]);
      unsigned long startMillis = millis();
      while (millis() - startMillis < pattern[n]) {}
      toggleWA(WA[i]);

      startMillis = millis();
      while (millis() - startMillis < 500) {}
    }
  }
}


void toggleWA(int input) {
  // Check and toggle atomizer.

  // Press switch once.`
  if (!WA_state[input-2]) {
    toggleAtomizer(input);
    WA_state[input-2] = true;
  } else {

    // Press switch thrice.
    for (int i = 0; i < 3; i++) {
      toggleAtomizer(input);
      unsigned long startMillis = millis();
      while (millis() - startMillis < 200) {}
    }
    WA_state[input-2] = false;
  }
}


void toggleAtomizer(int pin) {
  Serial.println("Toggle");
  digitalWrite(pin, HIGH);
  unsigned long startMillis = millis();
  while (millis() - startMillis < 200) {}
  digitalWrite(pin, LOW);
}
