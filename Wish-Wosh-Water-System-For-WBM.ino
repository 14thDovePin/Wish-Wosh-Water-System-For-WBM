// Dependencies and libraries for the SD Card Module.
#include <SPI.h>
#include <SD.h>

// Dependencies and libraries for the Temperature & Humidity Sensor.
#include <DHT.h>


// Program attributes & flags.
const int base = 2;  // Cycles per Second
const bool saveData = true;
const bool developerMode = false;


// Pinouts.
const int CPS_LED_IP = A3;  // CPS Indicator
const int A_LED_IP = A4;    // Red Indicator A
const int B_LED_IP = A5;    // Red Indicator B
const int P_OFF = A2;       // Power Off Button

// SD Card Module setup.
const int chipSelect = 10;  // Digital Pin

// Temperature & Humidity Sensor setup.
#define DHTPIN 9            // Digital Pin
#define DHTTYPE DHT11       // DHT Sensor Type
DHT dht(DHTPIN, DHTTYPE);   // Initialize class.

// Capacitive Soil Moisture Sensor setup.
const int DRY_VAL = 459;    // Callibration Value
const int WET_VAL = 913;    // Callibration Value
const int CSM1 = A7;        // Analog Pin


// Variables for CPS calculation.
const int mult = 1000;
const int cps = base * mult;
unsigned long previousMillis = 0;

unsigned long startTime;
String fileName;            // Current File


void setup() {
  // Setup serial monitor.
  Serial.begin(9600);

  if (saveData) {
    Serial.println("Save Data: Enabled");
  } else {
    Serial.println("Save Data: Disabled");
  }

  // Set digital pins.
  pinMode(CPS_LED_IP, OUTPUT);
  pinMode(A_LED_IP, OUTPUT);
  pinMode(B_LED_IP, OUTPUT);
  pinMode(P_OFF, INPUT_PULLUP);

  // Initialize modules and devices.
  sdcmInitialize();  // SD Card Module
  dht.begin();       // Temperature & Humidity Sensor
}


void loop() {
  // Calculate and check Cycles per Second.
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= cps) {
    previousMillis = currentMillis;
    cycle();
  }

  // Check power off trigger.
  if (digitalRead(P_OFF) == LOW) {
    powerOffLoop();
  }
}


void cycle() {
  // Tasks that needs to be done every cycle.
  cycleIndicator();

  // Check sd card initialization.
  check_sdcm();

  // Pull research data.
  String th = pd_th_sensor();    // Temperature & Humidity
  String sm = read_csm_data();   // Soil Moisture

  Serial.println(sm);
  // store_data(th);
}


void sdcmInitialize() {
  // Initialize the SD card module.
  if (SD.begin(chipSelect)) {
    Serial.println("Card initialized.");
  } else {
    Serial.println("Card failed.");
    sdcmError();
  }

  // Write the data file.
  if (saveData) {
    createCSVfile();
  }
}


void sdcmError() {
  // SD card module fail indicator.
  while (true) {
    digitalWrite(A_LED_IP, HIGH);
    delay(400);
    digitalWrite(A_LED_IP, LOW);
    delay(100);

    digitalWrite(B_LED_IP, HIGH);
    delay(400);
    digitalWrite(B_LED_IP, LOW);
    delay(100);
  }
}


void createCSVfile() {
  // Write the CSV file with its included headers into the SD card.
  int fileIndex = 0;
  String fileExtension = ".csv";

  // Construct a unique filename.
  while (SD.exists("data_" + String(fileIndex) + fileExtension)) {
    fileIndex++;
  }
  fileName = "data_" + String(fileIndex) + fileExtension;

  // Create csv file.
  File dataFile = SD.open(fileName, FILE_WRITE);

  if (dataFile) {
    // Write csv headers into the file.
    dataFile.print("Date,");
    dataFile.print("Time,");
    dataFile.print("Temperature,");
    dataFile.print("Humidity,");
    dataFile.print("Soil Moisture,");
    dataFile.println("Spray Count");

    // Close file.
    dataFile.close();
    Serial.println("File Created: `" + fileName + "`.");
  } else {
    Serial.println("Error Creating: `" + fileName + "`.");
    createFileError();
  }
}


void createFileError() {
  // File creation fail indicator.
  while (true) {
    digitalWrite(A_LED_IP, HIGH);
    delay(150);
    digitalWrite(A_LED_IP, LOW);
    delay(50);
    digitalWrite(A_LED_IP, HIGH);
    delay(150);
    digitalWrite(A_LED_IP, LOW);
    delay(100);

    digitalWrite(B_LED_IP, HIGH);
    delay(150);
    digitalWrite(B_LED_IP, LOW);
    delay(50);
    digitalWrite(B_LED_IP, HIGH);
    delay(150);
    digitalWrite(B_LED_IP, LOW);
    delay(100);
  }
}


void cycleIndicator() {
  // Toggles the CPS LED Indicator.
  digitalWrite(CPS_LED_IP, HIGH);
  delay(60);
  digitalWrite(CPS_LED_IP, LOW);
  delay(80);
  digitalWrite(CPS_LED_IP, HIGH);
  delay(60);
  digitalWrite(CPS_LED_IP, LOW);
}


void powerOffLoop() {
  // Run an infinite loop for safe powering off.
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
    sdcmError();
  }
}


String pd_th_sensor() {
  // Return data from DHT11 temperature & humidity sensor.
  float humidity = dht.readHumidity();
  // Callibrated according to weather & another
  // temperature measuring instrument.
  float temperature = dht.readTemperature() - 2;

  // Check if readings are valid.
  if (isnan(humidity) || isnan(temperature)) {
    pd_th_sensor_error();
    return String("None,None,");
  }

  // Convert temperature and humidity to strings with 2 decimal places.
  String th = String(temperature, 2);
  th += ',';
  th += String(humidity, 2);
  th += ',';

  return th;
}


void pd_th_sensor_error() {
  // Indicates temperature & humidity sensor error.
  Serial.println("TH Sensor Error");
  digitalWrite(A_LED_IP, HIGH);
  delay(200);
  digitalWrite(A_LED_IP, LOW);
  delay(600);
}


String read_csm_data() {
  // Read the analog value from the soil moisture sensor
  int csm1_raw = analogRead(CSM1);

  // Map the analog value to a percentage (0-100%)
  int csm1_value = map(csm1_raw, WET_VAL, DRY_VAL, 0, 100);

  // Print the moisture percentage to the serial monitor
  String data = "";
  data += String(csm1_value)+"%,";

  return data;
}


void csm_sensor_error_indicator_(float value, String sensor) {
  // Check the expected value returned by csm sensor.
  if (value < 0 || value > 100) {
  Serial.println("CSM " + sensor + "Error | Value unexpected!");
  digitalWrite(B_LED_IP, HIGH);
  delay(200);
  digitalWrite(B_LED_IP, LOW);
  delay(600);
  }
}


void store_data(String research_data) {
  // Store the research data in the sd card.

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
