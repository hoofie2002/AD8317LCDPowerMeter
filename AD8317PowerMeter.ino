// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(19, 23, 18, 17, 16, 15);


#define AD8317_INPUT 36
#define AD8137_VREF 25

float vout = 0.000; // Holds the true value of the ADC output voltage
float powerdB = 0.00; // Calculated power in dBm
float pWatts = 0.00; // Calculated power in watts
float vRMS = 0.0;

const float MV_DB_SLOPE         = 0.0220;       // Slope of AD8317 characteristics in -mV/dBm. Is the same for all BANDS.
const float dBm_at_0V = 18.000;                 // My Adjustment from 15 normal value calibrated against HP8620A
const float MIN_DETECT_POWER = -60.0;
const int NUM_SAMPLES = 2000;

void setup() {
  // put your setup code here, to run once:
  pinMode(AD8317_INPUT, INPUT);
  //pinMode(AD8137_VREF, INPUT);
  //analogSetVRefPin(AD8137_VREF);
  //analogSetAttenuation((adc_attenuation_t)0);   // 6dB range
  Serial.begin(115200);
  Serial.println();
  analogSetWidth(12);

  lcd.begin(16, 2);
  lcd.print("(c) VK6HIL");
  lcd.setCursor(0, 1);
  lcd.print("hoofie2002@gmail");
  delay(2000);
}

void loop() {
  // put your main code here, to run repeatedly:

  float cumulative = 0.0;

  for (int i = 0; i  <  NUM_SAMPLES; i++) {
    uint16_t rmV = analogReadMilliVolts(AD8317_INPUT);
    vout = ((float)rmV) / 1000;
    cumulative  = cumulative  + vout;
  }
  cumulative = cumulative / NUM_SAMPLES;
  powerdB = convertVoltageToDbm(cumulative);
  pWatts = convertDbmToMilliWatt(powerdB);
  vRMS = convertdBmToMilliVolt(powerdB);

  Serial.print(powerdB, 4);
  Serial.print(" ");
  Serial.print(pWatts, 4);
  Serial.print(" ");
  Serial.println(vRMS, 4);

  lcd.clear();

  lcd.setCursor(0, 0);
  if (powerdB  <  MIN_DETECT_POWER)  {
    lcd.print("No Signal");
    lcd.setCursor(0,1);
    lcd.print("< -60db detect");
  } else {
    if (powerdB > 0) lcd.print("+");
    lcd.print(powerdB, 1);
    lcd.print("dBm");

    //Draw a Bar for the Signal Strength between -60 and 0DB
    // We can go 10 spaces
    lcd.setCursor(0, 1);
    int len = ((MIN_DETECT_POWER * -1) - (powerdB  < 0 ? (powerdB * -1)  : powerdB)) / 6;
    Serial.println(len);
    for (int x = 0; x < len; x++) {
      lcd.print("\xff");
    }


    lcd.setCursor(10, 0);
    lcd.print(vRMS, 1);
    lcd.print("mV");

    lcd.setCursor(12, 1);
    if (pWatts < 0.1) {
      lcd.print(pWatts * 1000, 0);
      lcd.print("uW");
    } else  {
      lcd.print(pWatts, 0);
      lcd.print("mW");
    }

  }




  delay(1000);
}

float convertVoltageToDbm(float voltage)
{
  return dBm_at_0V - (voltage / MV_DB_SLOPE);
}

/* Convert dBm to mW */
float convertDbmToMilliWatt(float dBm)
{
  return pow(10, dBm / 10.0);
}

/* Convert dBm to rms voltage */
float convertdBmToMilliVolt(float dBm)
{
  float wortel = 0.223607;
  float kwadrt = dBm / 20.0;
  return (wortel * (pow(10.0, kwadrt))) * 1000;
}
