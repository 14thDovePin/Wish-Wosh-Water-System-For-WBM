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
const int CSM1 = A1;        // Analog Pin
const int CSM2 = A2;        // Analog Pin
const int CSM3 = A6;        // Analog Pin
const int CSM4 = A7;        // Analog Pin


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
  checkSDCM();

  // Pull research data.
  String th = pullTHData();    // Temperature & Humidity
  String sm = pullCSMData();   // Soil Moisture

  // Concatinate & store data.
  String final = th + sm;
  if (saveData) {
  storeData(final);
  }

  Serial.println(final);
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
  Serial.println("File or Disk Error..");
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
    dataFile.print("SoilMoisture_1234,");
    dataFile.println("SprayCount_1234");

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


void checkSDCM() {
  // Check SD card module.
  if (!SD.begin(chipSelect)) {
    sdcmError();
  }
}


String pullTHData() {
  // Return data from the temperature and humidity sensor.
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature() - 2;
  // Temperature callibrated according to weather report
  // and another temperature measuring instrument.

  // Check data validity.
  if (isnan(humidity) || isnan(temperature)) {
    invalidTHData();
    return String("None,None,");
  }

  // Convert data to strings with 2 decimal places.
  String th = "";
  th += String(temperature, 2) + ',';
  th += String(humidity, 2) + ',';

  return th;
}


void invalidTHData() {
  // Indicates temperature & humidity sensor or data error.
  Serial.println("TH Sensor||Data Error");
  digitalWrite(A_LED_IP, HIGH);
  delay(200);
  digitalWrite(A_LED_IP, LOW);
}


String pullCSMData() {
  // Read the analog value from the soil moisture sensor.
  int csm1_raw = analogRead(CSM1);
  int csm2_raw = analogRead(CSM2);
  int csm3_raw = analogRead(CSM3);
  int csm4_raw = analogRead(CSM4);

  // Map the analog value to a percentage. (0-100%)
  int csm1_value = map(csm1_raw, WET_VAL, DRY_VAL, 0, 100);
  int csm2_value = map(csm2_raw, WET_VAL, DRY_VAL, 0, 100);
  int csm3_value = map(csm3_raw, WET_VAL, DRY_VAL, 0, 100);
  int csm4_value = map(csm4_raw, WET_VAL, DRY_VAL, 0, 100);

  // List all values.
  int values[] = {
    csm1_value,
    csm2_value,
    csm3_value,
    csm4_value
    };

  // Check data validity (0%-100%) and store them as strings.
  String data = "";
  int count = sizeof(values) / sizeof(values[0]);
  for (int i=0; i < count; ++i) {
      if (values[i] > 100 || values[i] < 0) {
        // Renew string data & indicate error.
        data += "None,";
        invalidCSMData(String(i+1));
      } else {
        // Concatinate data.
        data += String(values[i]);
      }
  }

  return data;
}


void invalidCSMData(String n) {
  // Indicator light for invalid CSMS data.
  Serial.println("CSMS["+n+"] Error | Value unexpected!");
  digitalWrite(B_LED_IP, HIGH);
  delay(200);
  digitalWrite(B_LED_IP, LOW);
}


void storeData(String research_data) {
  // Open file.
  File dataFile = SD.open(fileName, FILE_WRITE);

  // Check data & write it into the file.
  if (dataFile) {
    dataFile.println(research_data);
  } else {
    sdcmError();
  }

  // Close file.
  dataFile.close();
}
