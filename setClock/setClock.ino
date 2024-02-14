// Clock Module Libraries & Dependencies
#include <Ds1302.h>


// Switches
const int cycleBase = 2;      // Cycles per Second
const bool setClock   = false;
const bool debugMode  = true;

// Pinouts
const int CPS_LED_IP  = A3;  // CPS Indicator
const int A_LED_IP    = A4;  // Red Indicator A
const int B_LED_IP    = A5;  // Red Indicator B
const int P_OFF       = A2;  // Power Off Button
const int CLK_PIN     = 6;   // CM - Clock
const int DATA_PIN    = 7;   // CM - Data
const int RST_PIN     = 8;   // CM - Reset

Ds1302 rtc(RST_PIN, CLK_PIN, DATA_PIN);

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


void setup() {
  // Initialize serial monitor.
  Serial.begin(9600);

  // Set pins.
  pinMode(CPS_LED_IP, OUTPUT);
  pinMode(A_LED_IP, OUTPUT);
  pinMode(B_LED_IP, OUTPUT);
  pinMode(P_OFF, INPUT_PULLUP);

  // Initialize modules.
  clockModuleInitialize();  // Clock Module

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

  // Print date and time.
  Serial.println(String(getCurrentDT()));

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
      // "%d/%d/%d %d:%d:%d",
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





// void setClockModuleTime() {
//     // Check for clock module power loss.
//     Serial.println(
//         "Clock module power loss detected!\n"
//         "Requesting time.."
//     );

//     // Send a time request to a PC.
//     Serial.println("REQ_TIME");

//     // Wait for a response.
//     while (!Serial.available()) {
//         clockModuleRequest();
//     }

//     // Read time from serial.
//     String receivedTime = Serial.readString();
//     Serial.println("Received time: " + receivedTime);

//     // Parse time and set the clock module's time.
//     Ds1302::DateTime dnt;

//     sscanf(
//         receivedTime.c_str(),
//         "%d/%d/%d %d:%d:%d",
//         &dnt.month,
//         &dnt.day,
//         &dnt.year,
//         &dnt.hour,
//         &dnt.minute,
//         &dnt.second
//     );

//     if (debugMode) {
//         Serial.println("Clock Module Debugging..");
//         Serial.println("Parsed DnT Values: ");
//         Serial.println("Month: " + String(dnt.month));
//         Serial.println("Day: " + String(dnt.day));
//         Serial.println("Year: " + String(dnt.year));
//         Serial.println("Hour: " + String(dnt.hour));
//         Serial.println("Minute: " + String(dnt.minute));
//         Serial.println("Second: " + String(dnt.second));
//     }

//     rtc.setDateTime(&dnt);

//     // Indicate successful request & process.
//     clockModuleSet();
// }






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
