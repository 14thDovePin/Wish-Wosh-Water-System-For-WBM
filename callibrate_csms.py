"""
This script is used to callibrate the CSM Sensor by acting
as a serial monitor so that the output of the Arduino can
be copied an feeded into a spreadsheet.
"""


import os
import serial


PORT = "COM4"
BAUD_RATE = 9600
OUTPUT_FILE = 'callibration_values.txt'


def log_data(count=20):
    """Logs lines of data from an arduino serial monitor."""
    # Open serial port
    ser = serial.Serial(PORT, BAUD_RATE)
    values = []

    for _ in range(count):
        # Read, decode, and trim whitespace.
        data = ser.readline().decode('utf-8').strip()

        # Store and print data.
        values.append(data+'\n')
        print(data)

    values.append('\n')
    return values


if __name__ == '__main__':
    print("Logging 20 units of data for both dry and wet setups.")
    print("\nPress Enter/Return to log the Dry Setup.")
    input()

    # Log dry values.
    dry_values = log_data()

    print("\nPress Enter/Return to log the Wet Setup.")
    input()

    # Log dry values.
    wet_values = log_data()

    # Write output file.
    final = dry_values + wet_values
    with open(OUTPUT_FILE, 'w') as f:
        f.writelines(final)

    print("\nDone! Exiting..")
