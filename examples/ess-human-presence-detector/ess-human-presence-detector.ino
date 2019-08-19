// this is a minimal version without error checking
// this is meant purely to exaplain the API, and its use is not recommended
// to use this in a serious application

#include <sensirion_ess.h>

SensirionESS ess;

#define SGP_WARMUP_COUNT 20

void setup()
{
  Serial.begin(9600);
  delay(1000); // let console settle

  ess.initSensors();
  if (ess.getProductType() != SensirionESS::PRODUCT_TYPE_SGP30) {
    Serial.print("SGP30 is required for this demo. Exiting\n");
    while (1) delay(1000); // wait foever
  }
  digitalWrite(SensirionESS::LED_YEL, HIGH);
}

void loop() {
  static int measurementCount = 0;
  static double eco2_threshold = 380;

  ess.measureIAQ();

  Serial.print(ess.getECO2());
  Serial.print(" ");
  Serial.print(eco2_threshold);
  Serial.print("\n");

  if (measurementCount > SGP_WARMUP_COUNT) {
    if (ess.getECO2() > eco2_threshold) {
      digitalWrite(SensirionESS::LED_RED, HIGH);
    } else {
      digitalWrite(SensirionESS::LED_RED, LOW);
    }
  } else {
    if (measurementCount == SGP_WARMUP_COUNT) {
      digitalWrite(SensirionESS::LED_YEL, LOW);
      eco2_threshold = ess.getECO2() * 1.2;

      Serial.print("Threshold set at: ");
      Serial.println(eco2_threshold);
    }
    measurementCount++;
  }

  delay(ess.remainingWaitTimeMS());
}
