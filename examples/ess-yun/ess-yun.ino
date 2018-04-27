#include <sensirion_ess.h>

#include <HttpClient.h>

SensirionESS ess;
HttpClient client;

// ADJUST HERE:
const char* DWEET_URL = "http://dweet.io/dweet/for/<THING>";

long lastDweetTimestamp;
const int DWEET_THRESHOLD = 2000;

const int CO2_THRESHOLD = 1000;

int sampleCount = 0;
const int INIT_SAMPLE_THRESHOLD = 15;

String createRequest(float tvoc, float eco2, float temp, float rh,
    int initState, int perspres)
{

  String s = DWEET_URL;
  s += "?temp=";      s += temp;
  s += "&rh=";        s += rh;
  s += "&tvoc=";      s += tvoc;
  s += "&eco2=";      s += eco2;
  s += "&perspres=";  s += perspres;
  s += "&initstate="; s += initState;
  return s;
}

void setup()
{
  SerialUSB.begin(9600);
  while (!SerialUSB) {} // wait for a serial connection

  SerialUSB.print("Connecting to bridge... ");
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  Bridge.begin();
  digitalWrite(13, HIGH);
  SerialUSB.println("done!");

  // First step is to initialize the sensors; this should only fail if
  // the board is defect, or the connection isn't working. Since there's nothing
  // we can do if this fails, the code will loop forever if an error is detected
  if (ess.initSensors() != 0) {
      SerialUSB.print("Error while initializing sensors: ");
      SerialUSB.print(ess.getError());
      SerialUSB.print("\n");
      while (1) { // loop forever
        delay(1000);
      }
  }

  // The SGP sensor has product type information and feature set stored
  // the following code reads it out, and prints it to the SerialUSB console.
  // This is purely to demo the function calls, and is not necessary to operate
  // the sensor
  int type = ess.getProductType();
  int fsVersion = ess.getFeatureSetVersion();

  SerialUSB.print((type == SensirionESS::PRODUCT_TYPE_SGP30) ? "SGP30" : "SGPC3");
  SerialUSB.print(" detected, running feature set version ");
  SerialUSB.println(fsVersion);

  lastDweetTimestamp = millis();
}

void loop() {
  float temp, rh, tvoc, eco2 = -1;

  if (ess.measureIAQ() != 0) {
    SerialUSB.print("Error while measuring IAQ: ");
    SerialUSB.print(ess.getError());
    SerialUSB.print("\n");
  } else {
    tvoc = ess.getTVOC();
    eco2 = ess.getECO2(); // SGP30 only
  }

  // only increment until it reaches the threshold
  if (sampleCount < INIT_SAMPLE_THRESHOLD) {
    ++sampleCount;
  }

  if (ess.measureRHT() != 0) {
    SerialUSB.print("Error while measuring RHT: ");
    SerialUSB.print(ess.getError());
    SerialUSB.print("\n");
  } else {
    temp = ess.getTemperature();
    rh = ess.getHumidity();
  }

  int personPresent = (eco2 > CO2_THRESHOLD) ? 1 : 0;
  int initState = (sampleCount >= INIT_SAMPLE_THRESHOLD);

  // Send data to dweet.io
  long tNow = millis();
  if ((tNow  - lastDweetTimestamp) >= DWEET_THRESHOLD) {
    if (client.ready()) {
      lastDweetTimestamp = tNow;
      String request = createRequest(tvoc, eco2, temp, rh, initState, personPresent);
      client.getAsynchronously(request);

      SerialUSB.println(request);
    }
  }

  delay(ess.remainingWaitTimeMS());
}
