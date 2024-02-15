// TH Sensor Module Libraries & Dependencies
#include <DHT.h>

// Switches
const int cycleBase = 2;  // Cycles per Second

// Pinouts.
const int CPS_LED_IP  = A3;  // CPS Indicator
const int A_LED_IP    = A4;  // Red Indicator A
const int B_LED_IP    = A5;  // Red Indicator B
const int P_OFF       = A2;  // Power Off Button
#define DHTPIN          9   // Digital Pin

// Temperature & Humidity Sensor setup.
#define DHTTYPE DHT11       // DHT Sensor Type
DHT dht(DHTPIN, DHTTYPE);   // Initialize class.

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
  dht.begin();  // Temperature & Humidity Sensor

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

  // Pull Data
  Serial.println(pullTHData());

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


String pullTHData() {
  // Return data from th sensor.

  // Grab data.
  float humidity = dht.readHumidity();
  // Temperature callibrated according to local weather
  // report and another temperature measuring instrument.
  float temperature = dht.readTemperature() - 2;

  // Check data validity.
  if (isnan(humidity) || isnan(temperature)) {
    invalidTHData();
    return String("None,None,");
  }

  // Convert data to strings with 2 decimal places.
  String thd = "";
  thd += String(temperature, 2) + ",";
  thd += String(humidity, 2) + ",";

  return thd;
}


void invalidTHData() {
  // Indicates temperature & humidity sensor or data error.
  Serial.println("TH Sensor Error | Data invalid.");
  digitalWrite(A_LED_IP, HIGH);
  delay(200);
  digitalWrite(A_LED_IP, LOW);
}
