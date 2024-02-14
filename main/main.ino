// Module Libraries & Dependencies


// Switches
const int cycleBase = 2;  // Cycles per Second

// Pinouts.
const int CPS_LED_IP  = A3;  // CPS Indicator
const int A_LED_IP    = A4;  // Red Indicator A
const int B_LED_IP    = A5;  // Red Indicator B
const int P_OFF       = A2;  // Power Off Button

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
