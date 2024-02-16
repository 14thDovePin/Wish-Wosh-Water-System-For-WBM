// Clock Module Libraries & Dependencies
#include <Ds1302.h>

// SD Card Module Libraries & Dependencies
#include <SPI.h>
#include <SD.h>

// TH Sensor Module Libraries & Dependencies
#include <DHT.h>


// Switches
const int cycleBase = 2;      // Cycles per Second
const bool setClock   = false;
const bool debugMode  = true;
const bool saveData = true;
const bool  callibrateCSMS  = false;
bool skipAtmozerTest = false;

// Pinouts
const int CPS_LED_IP  = A3;  // CPS Indicator
const int A_LED_IP    = A4;  // Red Indicator A
const int B_LED_IP    = A5;  // Red Indicator B
const int P_OFF       = A2;  // Power Off Button
const int CLK_PIN     = 6;   // CM - Clock
const int DATA_PIN    = 7;   // CM - Data
const int RST_PIN     = 8;   // CM - Reset
const int chipSelect  = 10;  // SDCM - CS
#define DHTPIN          9    // Digital Pin
const int CSM1        = A0;  // CSM Sensor 1
const int CSM2        = A1;  // CSM Sensor 2
const int CSM3        = A6;  // CSM Sensor 3
const int CSM4        = A7;  // CSM Sensor 4
const int WA[4] = {2, 3, 4, 5};  // Water Atmomizers 1-4

Ds1302 rtc(RST_PIN, CLK_PIN, DATA_PIN);

// Temperature & Humidity Sensor setup.
#define DHTTYPE DHT11 // Sensor Type
DHT dht(DHTPIN, DHTTYPE); // Initialize Class

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

// Water Atomizers State
bool WA_state[4] = {false, false, false, false};

// Cycles per Second Variables
const int ms = 1000;
const int cps = cycleBase * ms;
unsigned long previousTime = millis();

// Clock Variables
const static char* WeekDays[] = {
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday",
  "Sunday"
};

// File Management Variables
String dataFileName;  // Data File Name
File dataFile;        // Data File
String logFileName;   // Log File Name
File logFile;         // Log File



void setup() {
  // Initialize serial monitor.
  Serial.begin(9600);

  // Set pins.
  pinMode(CPS_LED_IP, OUTPUT);
  pinMode(A_LED_IP, OUTPUT);
  pinMode(B_LED_IP, OUTPUT);
  pinMode(P_OFF, INPUT_PULLUP);

  if (saveData) {
    Serial.println("Save Data: Enabled");
  } else {
    Serial.println("Save Data: Disabled");
  }

  // Initialize modules.
  clockModuleInitialize();  // Clock Module
  sdcmInitialize();  // SD Card Module
  dht.begin();  // Temperature & Humidity Sensor

  // Test water atomizers.
  if (!skipAtmozerTest) {
    testWA();
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
  dataFile.close();
  logFile.close();

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

  // Check SD card module.
  if (!SD.begin(chipSelect)) {
    sdcmError();
  }

  // Print date and time.
  Serial.println(String(getCurrentDT()));

  // Print time elapsed.
  Serial.println(String(millis()/1000));

  // Pull Data
  Serial.println(pullTHData());  // Temperature & Humidity
  Serial.println(pullCSMData());  // Soil Moisture
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



void clockModuleInitialize() {
  // Initialize the clock module.
  rtc.init();
  Serial.println("Clock Initialized!");

  // Check clock module power loss.
  if (rtc.isHalted() || setClock) {
    setClockModuleTime();
  }
}


void setClockModuleTime() {
  // Check for clock module power loss.
  Serial.println(
    "Clock module power loss detected!\n"
    "Requesting time.."
  );

  // Send a time request to a PC.
  Serial.println("REQ_TIME");

  // Wait for a response.
  while (!Serial.available()) {
    clockModuleRequest();
  }

  // Read time from serial.
  String receivedTime = Serial.readString();
  Serial.println("Received time: "+receivedTime);

  // Parse time and set the clock module's time.
  Ds1302::DateTime dnt;
  sscanf(
    receivedTime.c_str(),
      "%hhu/%hhu/%hhu %hhu:%hhu:%hhu",
      &dnt.month,
      &dnt.day,
      &dnt.year,
      &dnt.hour,
      &dnt.minute,
      &dnt.second
    );

  if (debugMode) {
    Serial.println("Clock Module Debugging..");
    Serial.println("Parsed DnT Values: ");
    Serial.println("Month: "+String(dnt.month));
    Serial.println("Day: "+String(dnt.day));
    Serial.println("Year: "+String(dnt.year));
    Serial.println("Hour: "+String(dnt.hour));
    Serial.println("Minute: "+String(dnt.minute));
    Serial.println("Second: "+String(dnt.second));

    uint8_t month, day, year, hour, minute, second;

    sscanf(
      receivedTime.c_str(),
      "%hhu/%hhu/%hhu %hhu:%hhu:%hhu",
      &month,
      &day,
      &year,
      &hour,
      &minute,
      &second
    );

    Serial.println("Parsed Raw Values: ");
    Serial.println("Month: "+String(month));
    Serial.println("Day: "+String(day));
    Serial.println("Year: "+String(year));
    Serial.println("Hour: "+String(hour));
    Serial.println("Minute: "+String(minute));
    Serial.println("Second: "+String(second));
  }

  rtc.setDateTime(&dnt);

  // Indicate successful request & process.
  clockModuleSet();
}


void clockModuleRequest () {
  // Inidicate Clock Module requesting time.
  digitalWrite(A_LED_IP, HIGH);
  delay(200);
  digitalWrite(A_LED_IP, LOW);
  delay(50);
  digitalWrite(A_LED_IP, HIGH);
  delay(200);
  digitalWrite(A_LED_IP, LOW);
  delay(50);

  digitalWrite(B_LED_IP, HIGH);
  delay(200);
  digitalWrite(B_LED_IP, LOW);
  delay(50);
  digitalWrite(B_LED_IP, HIGH);
  delay(200);
  digitalWrite(B_LED_IP, LOW);
  delay(50);
}


void clockModuleSet() {
  // Indicate successful request & process.
  for (int i=0; i<3; i++) {
    digitalWrite(CPS_LED_IP, HIGH);
    delay(300);
    digitalWrite(CPS_LED_IP, LOW);
    digitalWrite(A_LED_IP, HIGH);
    delay(300);
    digitalWrite(A_LED_IP, LOW);
    digitalWrite(B_LED_IP, HIGH);
    delay(300);
    digitalWrite(B_LED_IP, LOW);
  }
}


String getCurrentDT() {
  // Return the current date and time string.
  Ds1302::DateTime now;
  rtc.getDateTime(&now);
  String dnt = "";

  static uint8_t last_second = 0;
  if (last_second != now.second) {
    last_second = now.second;

    // Date & Time Format -> MM/DD/YYYY HH/MM/SS
    if (now.month < 9) dnt += "0";
    dnt += now.month;                // 01-12
    dnt += "/";
    if (now.day < 9) dnt += "0";
    dnt += now.day;                  // 01-31
    dnt += "/";
    dnt += "20";
    dnt += now.year;                 // 00-99

    dnt += ",";
    if (now.hour < 9) dnt += "0";
    dnt += now.hour;                 // 00-23
    dnt += ":";
    if (now.minute < 9) dnt += "0";
    dnt += now.minute;               // 00-59
    dnt += ":";
    if (now.second < 9) dnt += "0";
    dnt += now.second;               // 00-59
    dnt += ",";

    return dnt;
  } else {
    return "01/01/2000,00:00:00,";
  }
}



void sdcmInitialize() {
  // Initialize the SD card module.
  if (SD.begin(chipSelect)) {
    Serial.println("Card Initialized!");
  } else {
    Serial.println("Card Failed!");
    sdcmError();
  }

  // Write files.
  if (saveData) {
    createCSVfile();
  }
}


void createCSVfile() {
  // Write the CSV files with its included headers.
  int fileIndex = 0;
  String fileExtension = ".csv";

  // Designate a number for the filename.
  while (SD.exists("data_" + String(fileIndex) + fileExtension)) {
    fileIndex++;
  }

  // Create and open the data file.
  dataFileName = "data_" + String(fileIndex) + fileExtension;
  dataFile = SD.open(dataFileName, FILE_WRITE);

  // Check data the file.
  if (!dataFile) {
    Serial.println("File Error: `" + dataFileName + "`");
    createFileError();
  }
  Serial.println("File Created: `" + dataFileName + "`");

  // Write csv headers to the data file.
  dataFile.println(
  "Date,"
  "Time,"
  "TimeElapsed,"
  "Temperature,"
  "Humidity,"
  "SoilMoisture1,"
  "SoilMoisture2,"
  "SoilMoisture3,"
  "SoilMoisture4"
  );

  // Create and open the log file.
  logFileName = "log_" + String(fileIndex) + ".txt";
  logFile = SD.open(logFileName, FILE_WRITE);

  // Check the log file.
  if (!logFile) {
    Serial.println("File Error: `" + logFileName + "`");
    createFileError();
  }
  Serial.println("File Created: `" + logFileName + "`");
}


void createFileError() {
  // Indicate error.
  Serial.println("Failed to write/open file.");

  // Close possible open files.
  dataFile.close();
  logFile.close();

  while (true) {
    digitalWrite(A_LED_IP, HIGH);
    delay(175);
    digitalWrite(A_LED_IP, LOW);
    delay(50);
    digitalWrite(A_LED_IP, HIGH);
    delay(175);
    digitalWrite(A_LED_IP, LOW);
    delay(100);

    digitalWrite(B_LED_IP, HIGH);
    delay(400);
    digitalWrite(B_LED_IP, LOW);
    delay(100);
  }
}


void sdcmError() {
  if (!saveData) return;

  // Indicate module failure.
  Serial.println("SD Card Module Error");

  // Close possibble open files.
  dataFile.close();
  logFile.close();

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



void testWA () {
  Serial.println("Testing Water Atomizers.");
  int pattern[3] = {1000, 2000, 3000};
  delay(1500);

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

  // Press switch once.
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

