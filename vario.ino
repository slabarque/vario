#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <toneAC.h>

//#define DEBUG

const unsigned long SAMPLE_RATE = 250;
const unsigned long ALIVE_TIMEOUT = 5l * 60l * 1000l;  //5 minutes
const float DESC_RATE_METER_SECOND = 1;
const unsigned int DESC_FREQ = 300;
const unsigned long DESC_DURATION = 200;
const float ASC_RATE_METER_SECOND = 0.3;
const unsigned int ASC_BASE_FREQ = 600;
const unsigned int ASC_TOP_FREQ = 2000;
const unsigned int ASC_FREQ_INCR = 15;
const unsigned long ASC_DURATION = 130;
const unsigned int START_BEGIN_FREQ = 350;
const unsigned int START_END_FREQ = 600;
const unsigned long START_DURATION = 50;
const unsigned long START_STEP = 20;
const unsigned int ERR_FREQ = 250;
const unsigned long ERR_DURATION = 150;
const unsigned long ERR_PAUZE = 150;
const unsigned int ERR_REPEAT = 5;
const unsigned int SUCCESS_FREQ = 1500;
const unsigned long SUCCESS_DURATION = 100;
const unsigned long SUCCESS_PAUZE = 50;
const unsigned long SUCCESS_REPEAT = 2;

Adafruit_BMP280 bmp;  // use I2C interface
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

float old_pressure = 0;
float desc_rate = 0;
float asc_rate = 0;
bool error = false;
unsigned long lastAliveTime;

void playSound(unsigned int freq, unsigned long duration, unsigned long pauze = 0, uint8_t volume = 10) {
  toneAC(freq, volume, duration);
  delay(pauze);
  lastAliveTime = millis();
}

void playStartSound() {
  unsigned int freq = START_BEGIN_FREQ;
  while (freq < START_END_FREQ) {
    playSound(freq, START_DURATION, 0, 1);
    freq += START_STEP;
  }
}

void playSuccessSound() {
  for (int i = 0; i < SUCCESS_REPEAT; i++) {
    playSound(SUCCESS_FREQ, SUCCESS_DURATION, SUCCESS_PAUZE, 10);
  }
}

void playErrorSound() {
  for (int i = 0; i < ERR_REPEAT; i++) {
    playSound(ERR_FREQ, ERR_DURATION, ERR_PAUZE);
  }
}

void playAliveSound() {
  playSound(1200, 100, 150);
  playSound(400, 250);
}

float getPressureHectoPascal() {
  sensors_event_t pressure_event;
  bmp_pressure->getEvent(&pressure_event);
  return pressure_event.pressure;
}

void printValue(const String &label, float value, const String &unit) {
#ifdef DEBUG
  Serial.print(label);
  Serial.print(": ");
  Serial.print(value);
  Serial.print(" ");
  Serial.println(unit);
#endif
}

void printMsg(const String &message) {
#ifdef DEBUG
  Serial.println(message);
#endif
}

void setup() {
  Serial.begin(9600);

  printMsg("Starting...");
  playStartSound();

  if (!bmp.begin()) {
    printMsg("Error: could not find sensor. Check wiring.");
    playErrorSound();
    error = true;
  } else {
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,    /* Operating Mode. */
                    Adafruit_BMP280::SAMPLING_X2,    /* Temp. oversampling */
                    Adafruit_BMP280::SAMPLING_X16,   /* Pressure oversampling */
                    Adafruit_BMP280::FILTER_X16,     /* Filtering. */
                    Adafruit_BMP280::STANDBY_MS_63); /* Standby time. */

    old_pressure = getPressureHectoPascal();
    asc_rate = (-ASC_RATE_METER_SECOND / 9) * SAMPLE_RATE;
    desc_rate = (DESC_RATE_METER_SECOND / 9) * SAMPLE_RATE;
    printValue("start pressure", old_pressure, "hPA");
    printValue("asc_rate", asc_rate, "hPA");
    printValue("desc_rate", desc_rate, "dPA");
    printValue("ALIVE_TIMEOUT", ALIVE_TIMEOUT, "ms");

    playSuccessSound();
    printMsg("Success...");
  }
}

void loop() {
  if (!error) {
    float new_pressure = getPressureHectoPascal();
    float rate = (new_pressure - old_pressure) * 1000.0;
    old_pressure = new_pressure;
    printValue("pressure", new_pressure, "hPA");
    printValue("rate", rate, "dPA");
    printValue("millis() - lastAliveTime", millis() - lastAliveTime, "ms");
    if (rate < asc_rate) {
      float pitch = ASC_BASE_FREQ - (rate * ASC_FREQ_INCR);
      if (pitch > ASC_TOP_FREQ) {
        pitch = ASC_TOP_FREQ;
      }
      printMsg("Ascending");
      playSound((unsigned int)pitch, ASC_DURATION, SAMPLE_RATE - ASC_DURATION);
    } else if (rate > desc_rate) {
      printMsg("Descending");
      playSound(DESC_FREQ, DESC_DURATION, SAMPLE_RATE - DESC_DURATION, 6);
    } else if (millis() - lastAliveTime > ALIVE_TIMEOUT) {
      printMsg("I'm alive");
      printValue("millis() - lastAliveTime", millis() - lastAliveTime, "ms");
      playAliveSound();
    } else {
      delay(SAMPLE_RATE);
    }
  }
}