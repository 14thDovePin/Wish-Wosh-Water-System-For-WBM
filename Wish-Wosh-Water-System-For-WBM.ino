// Dependencies for SD Card Module.
#include <SPI.h>
#include <SD.h>

// Dependencies for the DHT Temperature & Humidity Sensor.
#include <DHT.h>


// User defined variables.
const int base = 2;  // Cycles per Second
const bool developerMode = false;


// Indicator LEDs.
const int CPS_LED_IP = A3;  // CPS Indicator
const int A_LED_IP = A4;    // Red Indicator A
const int B_LED_IP = A5;    // Red Indicator B

// Power off button.
const int P_OFF = A2;

// SD Card Module setup.
const int chipSelect = 10;  // D10

// DHT Temperature & Humidity Sensor setup.
#define DHTPIN 9       // D9
#define DHTTYPE DHT11  // DHT Sensor Type

DHT dht(DHTPIN, DHTTYPE);


// Pre-defined variables.
const int mult = 1000;
const int cps = base * mult;
unsigned long previousMillis = 0;

unsigned long startTime;
String fileName;


void setup() {
  // Setup serial monitor.
  Serial.begin(9600);

  if (developerMode) {
    Serial.println("Developer Mode: Active");
  } else {
    Serial.println("Developer Mode: Inactive");
  }

  // Setup indicator pins.
  pinMode(CPS_LED_IP, OUTPUT);
  pinMode(A_LED_IP, OUTPUT);
  pinMode(B_LED_IP, OUTPUT);

  // Setup power off pin.
  pinMode(P_OFF, INPUT_PULLUP);

  // Initialize SD card module.
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed.");
    sdcm_fail_indicator();
  }
  else {
    Serial.println("Card initialized.");
  }

  // Write the necessary file.
  if (!developerMode) {
    write_data();
  }

  // Initialize temperature and humidity sensor.
  dht.begin();
}


void loop() {
  // Get the current time
  unsigned long currentMillis = millis();

  // Check if time exceeded CPS.
  if (currentMillis - previousMillis >= cps) {
    // Save the current time.
    previousMillis = currentMillis;

    // Cycle indicator.
    if (!developerMode) {
      cycle_indicator();
    } else {
      dm_cycle_indicator();
      return;
    }

    // Check sd card initialization.
    check_sdcm();

    // Pull temp & humidity data.
    String th = pd_th_sensor();

    store_data(th);
  }

  // Safe power off button.
  if (digitalRead(P_OFF) == LOW) {
    poff_ready_indicator();
  }
}


void sdcm_fail_indicator() {
  // SD Card Module Failed Indicator
  while (true) {
    digitalWrite(A_LED_IP, HIGH);
    delay(450);
    digitalWrite(A_LED_IP, LOW);
    delay(50);

    digitalWrite(B_LED_IP, HIGH);
    delay(450);
    digitalWrite(B_LED_IP, LOW);
    delay(50);
  }
}


void write_data() {
  int fileIndex = 0;

  // Find and create a unique filename.
  while (SD.exists("data_" + String(fileIndex) + ".txt")) {
    fileIndex++;
  }
  fileName = "data_" + String(fileIndex) + ".txt";

  // Create file with the unique filename.
  File dataFile = SD.open(fileName, FILE_WRITE);

  // Write data headers into the file.
  if (dataFile) {
    dataFile.print("Time Elapsed,");
    dataFile.print("Temperature,");
    dataFile.print("Humidity,");
    dataFile.print("Soil Moisture,");
    dataFile.println("Spray Count");

  // Close file and record the start time.
    dataFile.close();
    startTime = millis();
    Serial.println("File Created: `" + fileName + "`.");
  } else {
    Serial.println("Error Creating: `" + fileName + "`.");
  }
}


void cycle_indicator() {
  // Toggles the CPS LED Indicator.
  digitalWrite(CPS_LED_IP, HIGH);
  delay(200);
  digitalWrite(CPS_LED_IP, LOW);
}


void dm_cycle_indicator() {
  // Developer mode cycle indicator.
  digitalWrite(CPS_LED_IP, HIGH);
  delay(200);
  digitalWrite(CPS_LED_IP, LOW);

  delay(100);

  digitalWrite(CPS_LED_IP, HIGH);
  delay(200);
  digitalWrite(CPS_LED_IP, LOW);
}


void poff_ready_indicator() {
  // Allows for safely turning off the device.
  digitalWrite(CPS_LED_IP, HIGH);
  while (true) {
    digitalWrite(A_LED_IP, HIGH);
    digitalWrite(B_LED_IP, HIGH);
    delay(300);

    digitalWrite(A_LED_IP, LOW);
    digitalWrite(B_LED_IP, LOW);
    delay(700);
  }
}


void check_sdcm() {
  // Check SD card module.
  if (!SD.begin(chipSelect)) {
    sdcm_fail_indicator();
  }
}


String pd_th_sensor() {
  // Return data from DHT11 temperature & humidity sensor.
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Check if readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("TH Sensor Error");
    pd_th_sensor_error();
    return;
  }

  String th;
  th = String(temperature);
  th += ',';
  th += String(humidity);
  th += ',';

  return th;
}


void pd_th_sensor_error() {
  // Indicates temperature & humidity sensor error.
  digitalWrite(A_LED_IP, HIGH);
  delay(200);
  digitalWrite(A_LED_IP, LOW);
  delay(600);
}


void store_data(String research_data) {
  // Store research data in the sd card.

  // Open file.
  File dataFile = SD.open(fileName, FILE_WRITE);

  // Write data into the file.
  if (dataFile) {
    dataFile.println(research_data);

  // Close file.
    dataFile.close();
    Serial.println(research_data);
  }
}

