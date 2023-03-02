from time import sleep

while(1):
    with open('/sys/devices/platform/soc/40015000.i2c/i2c-1/1-0077/iio:device0/in_temp_input', 'r') as ftemp:
        temperature = ftemp.readline()



    with open('/sys/devices/platform/soc/40015000.i2c/i2c-1/1-0077/iio:device0/in_pressure_input', 'r') as fpress:
        pressure = fpress.readline()



    with open('/dev/LCD1602', 'w') as flcd:
        flcd.write('Temp: ' + temperature[:2] + '.' + temperature[2:3] + '#Press: ' + pressure[:5])


    sleep(2)    

