import serial
import time

ser = serial.Serial("COM1",9600)

for l in "SET MODE (1,2) [P];":
    time.sleep(0.07)
    ser.write(l.encode())