import serial
import time
import datetime


# Set to the serial port your Arduino is connected to.
SERIAL_PORT = 'COM6'


def send_time_to_arduino(serial_port, current_time):
    with serial.Serial(serial_port, 9600, timeout=1) as ser:
        # Wait for the request from Arduino
        while True:
            if ser.readline().decode().strip() == "REQ_TIME":
                break

        # Send the current time to Arduino
        ser.write(current_time.encode())
        print(f"Sent time to Arduino: {current_time}")


if __name__ == "__main__":
    current_time = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    send_time_to_arduino(SERIAL_PORT, current_time)
