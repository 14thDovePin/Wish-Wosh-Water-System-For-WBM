// Dependencies and libraries for the SD Card Module.
#include <SPI.h>
#include <SD.h>

// Dependencies and libraries for the Clock Module.
#include <Ds1302.h>

// Dependencies and libraries for the Temperature & Humidity Sensor.
#include <DHT.h>


// Switches.
const int   cycleBase         = 2;      // Cycles per Second
const bool  saveData          = true;
bool        skipAtmozerTest   = true;
const bool  CSMScallibration  = false;
const bool  setClock          = false;
const bool  debugMode         = false;


// Pinouts.
const int CPS_LED_IP  = A3;  // CPS Indicator
const int A_LED_IP    = A4;  // Red Indicator A
const int B_LED_IP    = A5;  // Red Indicator B
const int P_OFF       = A2;  // Power Off Button

// SD Card Module setup.
const int chipSelect  = 10;  // Digital Pin

// Clock Codule setup.
const int CLK_PIN     = 6;   // Digital Pin
const int DATA_PIN    = 7;   // Digital Pin
const int RST_PIN     = 8;   // Digital Pin
Ds1302 rtc(RST_PIN, CLK_PIN, DATA_PIN);

const static char* WeekDays[] = {
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday",
  "Sunday"
};

// Temperature & Humidity Sensor setup.
#define DHTPIN          9   // Digital Pin
#define DHTTYPE DHT11       // DHT Sensor Type
DHT dht(DHTPIN, DHTTYPE);   // Initialize class.

// Capacitive Soil Moisture Sensor setup.
const int CSM1 = A0;        // Analog Pin
const int DRY_VAL1 = 497;   // Callibrate Value
const int WET_VAL1 = 877;   // Callibrate Value

const int CSM2 = A1;        // Analog Pin
const int DRY_VAL2 = 483;   // Callibrate Value
const int WET_VAL2 = 853;   // Callibrate Value

const int CSM3 = A6;        // Analog Pin
const int DRY_VAL3 = 488;   // Callibrate Value
const int WET_VAL3 = 871;   // Callibrate Value

const int CSM4 = A7;        // Analog Pin
const int DRY_VAL4 = 481;   // Callibrate Value
const int WET_VAL4 = 869;   // Callibrate Value

// Water Atomizer setup.
// Digital pins & their state.
const int WA[4] = {2, 3, 4, 5};
bool WA_state[4] = {false, false, false, false};


// Variables for CPS calculation.
const int mult = 1000;
const int cps = cycleBase * mult;
unsigned long previousMillis = 0;

// Variables for file management.
String dataFileName;  // Data File Name
File dataFile;        // Data File
String logFileName;   // Log File Name
File logFile;         // Log File


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
  pinMode(WA[0], OUTPUT);
  pinMode(WA[1], OUTPUT);
  pinMode(WA[2], OUTPUT);
  pinMode(WA[3], OUTPUT);

  // Initialize essential modules.
  sdcmInitialize();         // SD Card Module
  // clockModuleInitialize();  // Clock Moodule

  // Manage sensors and devices.
  dht.begin();              // Temperature & Humidity Sensor
  if (!skipAtmozerTest) {   // Test Water Atomizers
    skipAtmozerTest = true;
    testWA();
  }
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

  // Pull current date & time.
  // String dt = getCurrentDT();
  String dt = "None,None,";

  // Calculate time elapsed.
  String te = String(millis()/1000)+",";
  // String te = String((unsigned long)millis()/1000)+",";

  // Pull research data.
  String th = pullTHData();           // Temperature & Humidity
  String sm = pullCSMData();          // Soil Moisture
  String sc = "None,None,None,None";  // Spray Count

  // Store data.
  if (saveData) {
  storeData(dt, te, th, sm, sc);
  }
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
  return;  // TODO: Remove after testing.
  if (!saveData) return;
  Serial.println("SDCMError | File or disk failure.");

  // Close files.
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


void createCSVfile() {
  // Write the CSV file with its included headers into the SD card.
  int fileIndex = 0;
  String fileExtension = ".csv";

  // Construct a unique filename.
  while (SD.exists("data_" + String(fileIndex) + fileExtension)) {
    fileIndex++;
  }

  // Create and check csv file.
  dataFileName = "data_" + String(fileIndex) + fileExtension;
  dataFile = SD.open(dataFileName, FILE_WRITE);

  if (dataFile) {
    // Write csv headers into the file.
    dataFile.println(
    "Date,"
    "Time,"
    "TimeElapsed,"
    "Temperature,"
    "Humidity,"
    "SoilMoisture_1,"
    "SoilMoisture_2,"
    "SoilMoisture_3,"
    "SoilMoisture_4,"
    "SprayCounter_1,"
    "SprayCounter_2,"
    "SprayCounter_3,"
    "SprayCounter_4"
    );

    Serial.println("File Created: `" + dataFileName + "`.");
  } else {
    Serial.println("Error Creating: `" + dataFileName + "`.");
    createFileError();
  }

  // Create and check log file.
  logFileName = "log_" + String(fileIndex) + ".txt";
  logFile = SD.open(logFileName, FILE_WRITE);

  if (logFile) {
    Serial.println("File Created: `" + logFileName + "`.");
  } else {
    Serial.println("Error Creating: `" + logFileName + "`.");
    createFileError();
  }
}


void createFileError() {
  return;  // TODO: Remove after testing.
  // File creation fail indicator.

  // Close files.
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


void clockModuleInitialize() {
  // Initialize clock module.
  rtc.init();
  Serial.println("Clock Module Initialized..");

  // Check for clock module power loss.
  if (rtc.isHalted()) {
    setClockModuleTime();
  }
}


void setClockModuleTime() {
  // Check for clock module power loss.
  printLog(
    "Clock module power loss detected!\n"
    "Requesting time.."
  );

  // Send a request to the PC
  Serial.println("REQ_TIME");

  // Wait for the response from the PC
  while (!Serial.available()) {
    clockModuleRequest();
  }

  // Read time from Serial
  String receivedTime = Serial.readString();
  printLog("Received time: "+receivedTime);

  // Parse received time and set the clock module's time.
  Ds1302::DateTime dnt;
  sscanf(
    receivedTime.c_str(),
      "%hhu/%hhu/%hu %hhu:%hhu:%hhu",
      &dnt.month,
      &dnt.day,
      &dnt.year,
      &dnt.hour,
      &dnt.minute,
      &dnt.second
    );

  if (debugMode) {
    printLog("Clock Module Debugging..");
    printLog("Parsed DnT Values: ");
    printLog("Month: "+String(dnt.month));
    printLog("Day: "+String(dnt.day));
    printLog("Year: "+String(dnt.year));
    printLog("Hour: "+String(dnt.hour));
    printLog("Minute: "+String(dnt.minute));
    printLog("Second: "+String(dnt.second));

    uint8_t month, day, year, hour, minute, second;

    sscanf(
      receivedTime.c_str(),
      // "%d/%d/%d %d:%d:%d",
      "%hhu/%hhu/%hu %hhu:%hhu:%hhu",
      &month,
      &day,
      &year,
      &hour,
      &minute,
      &second
    );

    printLog("Parsed Raw Values: ");
    printLog("Month: "+String(month));
    printLog("Day: "+String(day));
    printLog("Year: "+String(year));
    printLog("Hour: "+String(hour));
    printLog("Minute: "+String(minute));
    printLog("Second: "+String(second));
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
  // Indicate Clock Module successful request & process.
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
    if (now.month < 10) dnt += "0";
    dnt += now.month;                // 01-12
    dnt += "/";
    if (now.day < 10) dnt += "0";
    dnt += now.day;                  // 01-31
    dnt += "/";
    dnt += "20";
    dnt += now.year;                 // 00-99

    dnt += ",";
    if (now.hour < 10) dnt += "0";
    dnt += now.hour;                 // 00-23
    dnt += ":";
    if (now.minute < 10) dnt += "0";
    dnt += now.minute;               // 00-59
    dnt += ":";
    if (now.second < 10) dnt += "0";
    dnt += now.second;               // 00-59
    dnt += ",";

    return dnt;
  } else {
    return "01/01/2000,00:00:00,";
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
  String thd = "";
  thd += String(temperature, 2) + ",";
  thd += String(humidity, 2) + ",";

  return thd;
}


void invalidTHData() {
  // Indicates temperature & humidity sensor or data error.
  printLog("TH Sensor Error | Data invalid.");
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
  int csm1_value = map(csm1_raw, WET_VAL1, DRY_VAL1, 0, 100);
  int csm2_value = map(csm2_raw, WET_VAL2, DRY_VAL2, 0, 100);
  int csm3_value = map(csm3_raw, WET_VAL3, DRY_VAL3, 0, 100);
  int csm4_value = map(csm4_raw, WET_VAL4, DRY_VAL4, 0, 100);

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
  if (CSMScallibration) {
    String raw_data = "";
    raw_data += String(csm1_raw) + ",";
    raw_data += String(csm2_raw) + ",";
    raw_data += String(csm3_raw) + ",";
    raw_data += String(csm4_raw) + ",";
    return raw_data;
  }

  return data;
}


void invalidCSMData(String n) {
  // Indicator light for invalid CSMS data.
  printLog("CSMS["+n+"] Error | Value unexpected!");
  digitalWrite(B_LED_IP, HIGH);
  delay(150);
  digitalWrite(B_LED_IP, LOW);
  delay(50);
}


void testWA () {
  Serial.println("Testing Water Atomizers.");
  int pattern[3] = {3000, 1000, 1000};

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
  int count = input - 1;

  // Press switch once.
  if (!WA_state[count]) {
    toggleAtomizer(count);
    WA_state[count] = true;
  } else {

    // Press switch thrice.
    for (int i = 0; i < 3; i++) {
      toggleAtomizer(count);
      unsigned long startMillis = millis();
      while (millis() - startMillis < 50) {}
    }
    WA_state[count] = false;
  }
}


void toggleAtomizer(int index) {
  digitalWrite(WA[index], HIGH);
  unsigned long startMillis = millis();
  while (millis() - startMillis < 100) {}
  digitalWrite(WA[index], LOW);
}


void storeData(
  String dt,
  String te,
  String th,
  String sm,
  String sc
  ) {
  // Check data & write it into the file.
  Serial.print(dt);
  Serial.print(te);
  Serial.print(th);
  Serial.print(sm);
  Serial.println(sc);

  if (dataFile) {
    dataFile.print(dt);
    dataFile.print(te);
    dataFile.print(th);
    dataFile.print(sm);
    dataFile.println(sc);
  } else {
    sdcmError();
  }

  // Flush data file.
  dataFile.flush();
}


void printLog(String text_input) {
  // Insert calculated time elapsed.
  String prefix = "[" + String(millis()/1000) + "] -> ";

  // Check data & write it into the file.
  if (logFile) {
    Serial.print(prefix);
    Serial.println(text_input);
    logFile.print(prefix);
    logFile.println(text_input);
  } else {
    sdcmError();
  }

  // Flush data.
  logFile.flush();
}
