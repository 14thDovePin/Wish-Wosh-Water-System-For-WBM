// Dependencies and libraries for the SD Card Module.
#include <SPI.h>
#include <SD.h>

// Dependencies and libraries for the Clock Module.
#include <Ds1302.h>

// Dependencies and libraries for the Temperature & Humidity Sensor.
#include <DHT.h>


// Program attributes & flags.
const int base = 2;  // Cycles per Second
const bool debugMode = true;
const bool saveData = true;
const bool setClock = false;
const bool callibrateCSMS = false;


// Pinouts.
const int CPS_LED_IP = A3;  // CPS Indicator
const int A_LED_IP = A4;    // Red Indicator A
const int B_LED_IP = A5;    // Red Indicator B
const int P_OFF = A2;       // Power Off Button

// SD Card Module setup.
const int chipSelect = 10;  // Digital Pin

// Clock Codule setup.
const int CLK_PIN = 6;      // CLK  ->  Digital Pin
const int DATA_PIN = 7;     // DATA ->  Digital Pin
const int RST_PIN = 8;      // RST  ->  Digital Pin
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
#define DHTPIN 9            // Digital Pin
#define DHTTYPE DHT11       // DHT Sensor Type
DHT dht(DHTPIN, DHTTYPE);   // Initialize class.

// Capacitive Soil Moisture Sensor setup.
const int CSM1 = A0;        // Analog Pin
const int DRY_VAL1 = 459;   // Callibrate Value
const int WET_VAL1 = 913;   // Callibrate Value

const int CSM2 = A1;        // Analog Pin
const int DRY_VAL2 = 459;   // Callibrate Value
const int WET_VAL2 = 913;   // Callibrate Value

const int CSM3 = A6;        // Analog Pin
const int DRY_VAL3 = 459;   // Callibrate Value
const int WET_VAL3 = 913;   // Callibrate Value

const int CSM4 = A7;        // Analog Pin
const int DRY_VAL4 = 459;   // Callibrate Value
const int WET_VAL4 = 913;   // Callibrate Value


// Variables for CPS calculation.
const int mult = 1000;
const int cps = base * mult;
unsigned long previousMillis = 0;

unsigned long startTime;
String fileName;            // Current File
String logFileName;         // Current Log File


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

  // Initialize essential modules.
  sdcmInitialize();         // SD Card Module
  clockModuleInitialize();  // Clock Moodule

  // Manage sensors and devices.
  dht.begin();              // Temperature & Humidity Sensor
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

  // Print current date & time.
  printLog(getCurrentDT());

  // Pull research data.
  String th = pullTHData();    // Temperature & Humidity
  String sm = pullCSMData();   // Soil Moisture

  // Concatinate & store data.
  String final = th + sm;
  if (saveData) {
  storeData(final);
  }

  // printLog(final);
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
  if (!saveData) return;
  Serial.println("SDCMError | File or disk failure.");
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
  fileName = "data_" + String(fileIndex) + fileExtension;
  File dataFile = SD.open(fileName, FILE_WRITE);

  if (dataFile) {
    // Write csv headers into the file.
    String headers = "";
    headers += "Date,";
    headers += "Time,";
    headers += "TimeElapsed,"
    headers += "Temperature,";
    headers += "Humidity,";
    headers += "SoilMoisture_1,";
    headers += "SprayCounter_1";
    headers += "SoilMoisture_2,";
    headers += "SprayCounter_2";
    headers += "SoilMoisture_3,";
    headers += "SprayCounter_3";
    headers += "SoilMoisture_4,";
    headers += "SprayCounter_4";
    dataFile.println(headers);

    dataFile.close();
    printLog("File Created: `" + fileName + "`.");
  } else {
    Serial.println("Error Creating: `" + fileName + "`.");
    createFileError();
  }

  // Create and check log file.
  logFileName = "log_" + String(fileIndex) + ".txt";
  File logFile = SD.open(logFileName, FILE_WRITE);

  if (logFile) {
    logFile.close();
    printLog("File Created: `" + logFileName + "`.");
  } else {
    Serial.println("Error Creating: `" + logFileName + "`.");
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


void clockModuleInitialize() {
  // Initialize clock module.
  rtc.init();
  printLog("Clock Module Initialized..");

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
  printLog("REQ_TIME");

  // Wait for the response from the PC
  while (!Serial.available()) {
    clockModuleRequest();
  }

  // Read time from Serial
  String receivedTime = Serial.readString();
  printLog("Received time: "+receivedTime);

  // Parse received time and set the clock module's time.
  Ds1302::DateTime dt;
  sscanf(
    receivedTime.c_str(),
      "%hhu/%hhu/%hu %hhu:%hhu:%hhu",
      &dt.month,
      &dt.day,
      &dt.year,
      &dt.hour,
      &dt.minute,
      &dt.second
    );

  if (debugMode) {
    printLog("Clock Module Debugging..");
    printLog("Parsed DT Values: ");
    printLog("Month: "+String(dt.month));
    printLog("Day: "+String(dt.day));
    printLog("Year: "+String(dt.year));
    printLog("Hour: "+String(dt.hour));
    printLog("Minute: "+String(dt.minute));
    printLog("Second: "+String(dt.second));

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

  rtc.setDateTime(&dt);

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
  String dt = "";

  static uint8_t last_second = 0;
  if (last_second != now.second) {
    last_second = now.second;

    // Date & Time Format -> MM/DD/YYYY HH/MM/SS
    if (now.month < 10) dt += "0";
    dt += now.month;                // 01-12
    dt += "/";
    if (now.day < 10) dt += "0";
    dt += now.day;                  // 01-31
    dt += "/";
    dt += "20";
    dt += now.year;                 // 00-99

    dt += ",";
    if (now.hour < 10) dt += "0";
    dt += now.hour;                 // 00-23
    dt += ":";
    if (now.minute < 10) dt += "0";
    dt += now.minute;               // 00-59
    dt += ":";
    if (now.second < 10) dt += "0";
    dt += now.second;               // 00-59

    return dt;
  } else {
    return "01/01/2000,00:00:00";
  }
}


uint8_t parseDigits(char* str, uint8_t count) {
  // Parse digits for the clock module.
  uint8_t val = 0;
  while(count-- > 0) val = (val * 10) + (*str++ - '0');
  return val;
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
  th += String(temperature, 2) + ",";
  th += String(humidity, 2) + ",";

  return th;
}


void invalidTHData() {
  // Indicates temperature & humidity sensor or data error.
  printLog("TH Sensor||Data Error");
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
  String err_msg;
  // Check data validity (0%-100%) and store them as strings.
  for (int i=0; i < 4; ++i) {
    int mapped_data = values[i];
    if (mapped_data > 100 || mapped_data < 0) {
      // Renew string data & indicate error.
      data += "None,";
      invalid = true;
      err_msg = String(i+1);
    } else {
      // Concatinate data.
      data += String(mapped_data)+",";
    }
  }

  // Handle error.
  if (invalid) {
    invalidCSMData(err_msg);
  }

  // Callibrate CSM sensor.
  if (callibrateCSMS) {
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


void printLog(String text) {
  // Open log file.
  File logFile = SD.open(logFileName, FILE_WRITE);

  // Check data & write it into the file.
  if (logFile) {
    Serial.println(text);
    logFile.println(text);
  } else {
    sdcmError();
  }

  // Close file.
  logFile.close();
}
