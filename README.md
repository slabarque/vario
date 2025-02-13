# Vario
A basic variometer using a Arduino Nano and a BMP280 sensor.
Based on the [BMP280](https://github.com/danheeks/vario/tree/master/bmp280piezo) design in [danheek](https://github.com/danheeks)'s [vario](https://github.com/danheeks/vario) project.

I reorganized the original code a bit and added a _"I'm alive"_ signal every 5min to prevent battery drain should I forget to switch it off.

It's important to note that the Arduino Nano should be based on the ATmega328 microcontroller, the newer Arduino Nano Every is not compatible with some of the libraries used (toneAC, Adafruit_BMP280).