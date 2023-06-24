import serial
import csv
import time
import statistics as stats


port = 'COM5'
baudrate = 57600
serial = serial.Serial(port, baudrate)

freq_10 = 0
freq_60 = 0
data = []
frequencies = []
i = 0

# previous_time = time.time()
previous_time = -1

try:
    while True:
        try:
            new_line = float(serial.readline().decode().strip())
            # new_line = serial.readline().decode().strip()
            current_time = time.time()
            print(new_line)

            if isinstance(new_line, float):
                data.append(new_line) 
                frequencies.append(1 / (current_time - previous_time))
                previous_time = current_time

            print(f"Last 100 frequency is {stats.mean(frequencies[-100:])}") if ((i+1) % 100 == 0) else None
            i += 1
        except ValueError:
            print('Could not convert string to float')

except KeyboardInterrupt:   # Code execution will break when Ctrl+C is pressed
    print('Program stopped by user')
    serial.close()

# Write the data to a .csv file
file = open('values.csv', 'w')
writer = csv.writer(file)
writer.writerows(data)
file.close()