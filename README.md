# Wish Wosh Water System For WBMs (WWWS-WBM)
The **Wish Wosh Water System For White Button Mushrooms** is an automated sprinkler system designed to log research data and automate watering in mushroom cultivation, specifically *White Button Mushrooms* (WBM).

This system was designed for a group research project of *14thDovePin* back in his final year of high school and its purpose is purely for referencing the source code for the micro controller used, in this case an Arduino Nano.

## Research Data
This setup is only designed to log the following data while cultivating WBMs:
- Date
- Time
- Relative Humidity
- Temperature
- Soil Moisture Content
- Spray Count

## Indicator Lights
There are 3 indicator lights accordingly as followed:  
[B] [R<sub>A</sub>] [R<sub>B</sub>]

[B] = Cycle per Second Indicator Light  
[R<sub>A</sub>] = Red Indicator Light A  
[R<sub>B</sub>] = Red Indicator Light B

, = Short Delay  
... = Long Delay  
--- = Repeat

**Normal Cycles** - All systems normal.  
[B] , [B] ... [B] , [B] ---

**SD Card Error**  
[R<sub>A</sub>] , [R<sub>B</sub>] , [R<sub>A</sub>]  ,   [R<sub>B</sub>] ---

**Temperature & Humidity Sensor Error**  
[R<sub>A</sub>] , [R<sub>A</sub>] ---

**Soil Moisture Content Sensor Error**  
[R<sub>B</sub>] ... [R<sub>B</sub>] ---

**Shutdown Ready** - Safe to power off.  
[B] [B] ...  
[R<sub>A</sub>] [R<sub>B</sub>] ... [R<sub>A</sub>] [R<sub>B</sub>] ---

## Footnotes
- The IC of the RTC Module is a`DS1302`.
- Title courtesy of ***Snowphira***.
