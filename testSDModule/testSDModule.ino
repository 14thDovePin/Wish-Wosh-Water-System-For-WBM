// SD Card Module Libraries & Dependencies
#include <SPI.h>
#include <SD.h>


// Switches
const bool saveData = true;

// Pinouts.
const int chipSelect  = 10;  // SDCM - CS
const int CPS_LED_IP  = A3;  // CPS Indicator
const int A_LED_IP    = A4;  // Red Indicator A
const int B_LED_IP    = A5;  // Red Indicator B

// File Management Variables
String dataFileName;  // Data File Name
File dataFile;        // Data File
String logFileName;   // Log File Name
File logFile;         // Log File


void setup() {
  // Initialize serial monitor.
  Serial.begin(9600);

  // Set pins.
  pinMode(A_LED_IP, OUTPUT);
  pinMode(B_LED_IP, OUTPUT);

  if (saveData) {
    Serial.println("Save Data: Enabled");
  } else {
    Serial.println("Save Data: Disabled");
  }

  // Initialize modules.
  sdcmInitialize();  // SD Card Module

  Serial.println("Test Finished!");
}


void loop() {}


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
  "SoilMoisture_1,"
  "SoilMoisture_2,"
  "SoilMoisture_3,"
  "SoilMoisture_4,"
  "SprayCounter_1,"  // TODO: remove spray counter
  "SprayCounter_2,"
  "SprayCounter_3,"
  "SprayCounter_4"
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
