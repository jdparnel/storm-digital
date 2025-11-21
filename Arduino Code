//sold to mid american
#define pversion "Digital SN_7 7.11.25"

#include <SPI.h>
#include <Wire.h>  // For I2C Communication SDA & SCL
#include <GD2.h>
#include <RTClib.h>
RTC_PCF8523 rtc;

#include <Adafruit_ADS1X15.h>  //Library for 16Bit ADC
Adafruit_ADS1115 ads1;
Adafruit_ADS1115 ads2;

#include <MAX31856.h>
#define NUM_MAX31856 2
//MAX31856(sdi/mosi, sdo/miso, cs, clk)
MAX31856 *TemperatureSensor[NUM_MAX31856] = { new MAX31856(42, 41, 39, 40), new MAX31856(5, 6, 11, 7) };

#define CR0_INIT (CR0_AUTOMATIC_CONVERSION + CR0_OPEN_CIRCUIT_FAULT_TYPE_K /* + CR0_NOISE_FILTER_50HZ */)
#define CR1_INIT (CR1_AVERAGE_2_SAMPLES + CR1_THERMOCOUPLE_TYPE_K)
#define MASK_INIT (~(MASK_VOLTAGE_UNDER_OVER_FAULT + MASK_THERMOCOUPLE_OPEN_FAULT))

#include <EEPROMex.h>
int address = 0;  //starting address for eepromex
int dp10zeroadr = EEPROM.getAddress(sizeof(float));
int dp24zeroadr = EEPROM.getAddress(sizeof(float));
int dp138zeroadr = EEPROM.getAddress(sizeof(float));
int dp01zeroadr = EEPROM.getAddress(sizeof(float));
int dp05zeroadr = EEPROM.getAddress(sizeof(float));   //why int and not float?
int dprecadr = EEPROM.getAddress(sizeof(int));        //record time address
int dpsensoradr = EEPROM.getAddress(sizeof(int));     //dp sensor selection address
int spsensoradr = EEPROM.getAddress(sizeof(int));     //sp sensor selection address
int nullsensoradr = EEPROM.getAddress(sizeof(int));   //null sensor selection address
int pitchsensoradr = EEPROM.getAddress(sizeof(int));  //pitch sensor selection address

byte value;  //this is used for finding the used bytes in eeprom, the code is commented out under void loop()

int dapoint = 1, daport = 1, dapipe = 1, mydapipe, mydaport;
float cirvolt = 5;
float BP, DP01, DP05, DP10, DP24, DP138, TPE1, TPI1, TPE2, TPI2, DP05AVG, DP01AVG, DP10AVG, DP24AVG, DP138AVG, TPAVG;
float DP, StatP, Null, Pitch1;          //variables for displaying the raw values to screen
float DPAVG, SPAVG, NullAVG, PitchAVG;  //variable for displaying the average values to the screen
float bpoffset = 0, dp10z, dp24z, dp138z, dp01z, dp05z;
float BPU;
int DApipes = 8, DAports = 3;  //these two values MUST be the same as the below array index to automatically fix the DA code to work for the respected [pipes] & [ports] (note the DA summary page will have to be manually adjusted if 3 ports are NOT used)
float DA[8][3][12];            //DA [Pipes][Ports][Points]

int papoint = 1, patest = 1, paport = 1, mypatest, mypaport;  //variables for the PA page (mypaxxx is used for the index of the array)
int PAtests = 1, PAports = 6;                                 //these two values MUST be the same as the below array index to automatically fix the PA code to work for the respected [pipes] & [ports]
float PAVH[1][6][10], PASP[1][6][10], PATP[1][6][10];         //PAXX[Tests][Ports][Points]

int16_t adc0, adc1, adc2, adc3, adc4, adc5, adc8;
float adcv0, adcv1, adcv2, adcv3, adcv4, adcv5, adcv8;
float multiplier = 0.1875F / 1000;  //same as 6.144/32767
char bpv[16], dp01v[16], dp10v[16], dp24v[16], dp138v[16], tpv[16], tpi[16], tpv1[16], tpvi1[16], tpvi[16], tpi1[16], dp10a[16], dp24a[28], dp138a[16], dpa[16], spa[16], nulla[16], pitcha[16];
char tpvi0[16];

int dprec = 0, tprec = 0;
int dpsensor, spsensor, nullsensor, pitchsensor;
int page = 1;
int secnow = 0, minnow = 0, minprev = 0;
int secprev = 0, secelps = 0;
int secrec, rectime;
int sensorzero = 0;
int rec = 0;
int samples = 0;

int ledPin1 = 36;
int ledPin2 = 37;


#include <Average.h>  //For average function
#include "RunningAverage.h"
RunningAverage myBP(20);        //Interal dampening, running average
RunningAverage myDP01(10);      //Interal dampening, running average
RunningAverage myDP05(10);      //Internal dampening, running average
RunningAverage myDP10(10);      //Interal dampening, running average
RunningAverage myDP24(10);      //Interal dampening, running average
RunningAverage myDP138(10);     //Interal dampening, running average
RunningAverage myRADP01(10);    //running average
RunningAverage myRADP05(10);    //running average
RunningAverage myRADP10(10);    //running average
RunningAverage myRADP24(10);    //running average
RunningAverage myRADP138(10);   //running average
RunningAverage myRATP(10);      //running average
RunningAverage myRADP01Z(20);   //for zeroing
RunningAverage myRADP05Z(20);   //for zeroing
RunningAverage myRADP10Z(20);   //for zeroing
RunningAverage myRADP24Z(20);   //for zeroing
RunningAverage myRADP138Z(20);  //for zeroing
RunningAverage myRAcirvolt(20);  //for zeroing

//
unsigned long millisprev;
unsigned long startMillis, currentMillis, seconds, elpsMillis = 0, lastMillis = 0;
unsigned long tm;
char timestampl[30];


void setup() {
  Serial.println("Entering Setup");
  Serial.begin(9600);
  //  Serial1.begin(115200);
  GD.begin(0);
  GD.cmd_setrotate(0);
  //  ads1.begin(); ads2.begin();
  ads1.begin(0x48);
  ads2.begin(0x49);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);

  for (int i = 0; i < NUM_MAX31856; i++) {
    TemperatureSensor[i]->writeRegister(REGISTER_CR0, CR0_INIT);
    TemperatureSensor[i]->writeRegister(REGISTER_CR1, CR1_INIT);
    TemperatureSensor[i]->writeRegister(REGISTER_MASK, MASK_INIT);
  }

   // GD.self_calibrate(); //Uncomment if seems like touch is wrong!!

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
   // while (1)  ;
  }
  if (!rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    // while (1);
  }

  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); //Uncomment to set time
  //   rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0)); //Uncomment to manually set time

  DateTime now = rtc.now();
  secprev = now.second();
  minprev = now.minute();
  millisprev = millis();
  rectime = secrec;

  Serial.println("EEPROMex Address ");
  Serial.print("dp01zeroadr :");
  Serial.println(dp01zeroadr);
  Serial.print("dp05zeroadr :");
  Serial.println(dp01zeroadr);
  Serial.print("dp10zeroadr :");
  Serial.println(dp10zeroadr);
  Serial.print("dp24zeroadr :");
  Serial.println(dp24zeroadr);
  Serial.print("dp138zeroadr :");
  Serial.println(dp138zeroadr);
  Serial.print("dpsensoradr :");
  Serial.println(dpsensoradr);
  Serial.print("spsensoradr :");
  Serial.println(spsensoradr);
  Serial.print("nullsensoradr :");
  Serial.println(nullsensoradr);
  Serial.print("pitchsensoradr :");
  Serial.println(pitchsensoradr);
  Serial.print("dprecadr :");
  Serial.println(dprecadr);
  Serial.print(value, DEC);

  Serial.print("DP 01 Zero Value - EEPROM: ");
  Serial.println(EEPROM.readFloat(dp01zeroadr), 4);
  dp01z = EEPROM.readFloat(dp01zeroadr), 4;
  Serial.print("DP 05 Zero Value - EEPROM: ");
  Serial.println(EEPROM.readFloat(dp05zeroadr), 4);
  dp05z = EEPROM.readFloat(dp05zeroadr), 4;
  Serial.print("DP 10 Zero Value - EEPROM: ");
  Serial.println(EEPROM.readFloat(dp10zeroadr), 4);
  dp10z = EEPROM.readFloat(dp10zeroadr), 4;
  Serial.print("DP 24 Zero Value - EEPROM: ");
  Serial.println(EEPROM.readFloat(dp24zeroadr), 4);
  dp24z = EEPROM.readFloat(dp24zeroadr), 4;
  Serial.print("DP 138 Zero Value - EEPROM: ");
  Serial.println(EEPROM.readFloat(dp138zeroadr), 4);
  dp138z = EEPROM.readFloat(dp138zeroadr), 4;
  Serial.print("DP Active Sensor - EEPROM: ");
  Serial.println(EEPROM.readInt(dpsensoradr));
  dpsensor = EEPROM.readInt(dpsensoradr);
  Serial.print("SP Active Sensor - EEPROM: ");
  Serial.println(EEPROM.readInt(spsensoradr));
  spsensor = EEPROM.readInt(spsensoradr);
  Serial.print("Null Active Sensor - EEPROM: ");
  Serial.println(EEPROM.readInt(nullsensoradr));
  nullsensor = EEPROM.readInt(nullsensoradr);
  Serial.print("Pitch Active Sensor - EEPROM: ");
  Serial.println(EEPROM.readInt(pitchsensoradr));
  pitchsensor = EEPROM.readInt(pitchsensoradr);
  Serial.print("DP Record Time - EEPROM: ");
  Serial.println(EEPROM.readInt(dprecadr));
  secrec = EEPROM.readInt(dprecadr);
  rectime = secrec;

  Serial.println(timestampl);
  Serial.print("DP Sensor: ");
  Serial.println(dpsensor);
  Serial.print("SP Sensor: ");
  Serial.println(spsensor);
  Serial.print("Null Sensor: ");
  Serial.println(nullsensor);
  Serial.print("Pitch Sensor: ");
  Serial.println(pitchsensor);
}

void loop() {
  DateTime now = rtc.now();
  secnow = now.second();
  minnow = now.minute();

  if (page == 1) {  //Home Page
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
    Page1();
  } else if (page == 2) {  //Settings Page
    Page2();
  } else if (page == 3) {  //Sensor Setting Page
    Page3();
  } else if (page == 11) {  //Barometric pressure Page
    myBP.clear();
    Page11();
  } else if (page == 12) {  //Temperature pressure Page
    // Serial.println("Page 12 - Temperature "); //prints every loop
    myRATP.clear();  //clears every loop
    Page12();
  } else if (page == 13) {  //differential pressure Page
    Page13();
  } else if (page == 21) {  //Primary Air Page
    Page21();
  } else if (page == 23) {  //Dirty Air Page
    Page23();
  } else if (page == 233) {  //Dirty Air Multiple Ports Summary
    Page233();
  } else if (page == 31) {  //Fecheimer Page
    Page31();
  }
}

/*//////////////////Header for all pages///////////////////////////////////////////////////////////////////////*/
void header() {
  GD.cmd_text(GD.w / 2, 2, 31, OPT_CENTERX, "STORM TECHNOLOGIES");
  DateTime now = rtc.now();
 // char date[30];
 // sprintf(date, "%2d/%2d/%2d", now.month(), now.day(), now.year());
 // char time[30];
 // sprintf(time, "%02d:%02d", now.hour(), now.minute());
 // GD.cmd_text(725, 10, 28, OPT_CENTERX, date);
 // GD.cmd_text(725, 36, 28, OPT_CENTERX, time);  //displays time on screen
  //timestamp long (date and time)
 // sprintf(timestampl, "%2d/%2d/%2d %02d:%02d:%02d", now.month(), now.day(), now.year(), now.hour(), now.minute(), now.second());
}

/*//////////////////Home Page - Page 0///////////////////////////////////////////////////////////////////////*/
void Page1() {
  // Serial.print("Page 0 - Home ");
  GD.ClearColorRGB(0x550000);
  GD.Clear();
  header();
  GD.get_inputs();
  //GD.cmd_text(GD.w / 2, 40, 29 , OPT_CENTERX ,  "Multifunction Digital V_CB_3.1");
  GD.cmd_text(GD.w / 2, 40, 29, OPT_CENTERX, pversion);
  int x = 10, y = 100, w = 240, h = 100;
  int xs = w + 25, ys = h + 25;
  GD.Tag(2);  //settings
  GD.cmd_button(20, 20, 100, 50, 30, 0, "SET");
 // GD.Tag(11);                                         //label then button
 // GD.cmd_button(x, y, w, h, 30, 0, "Baro Pressure");  //x,y,w,h,font size,options,label
  GD.Tag(12);                                         //label then button
  GD.cmd_button(x, y + ys, w, h, 30, 0, "Temperature");
  GD.Tag(13);  //label then button
  GD.cmd_button(x, y + ys * 2, w, h, 30, 0, "Diff Pressure");
  GD.Tag(21);  //label then button
  GD.cmd_button(x + xs, y, w, h, 30, 0, "Primary Air");
 // GD.Tag(22);  //label then button
 // GD.cmd_button(x + xs, y + ys, w, h, 30, 0, "Static & Temp");
  GD.Tag(23);  //label then button
  GD.cmd_button(x + xs, y + ys * 2, w, h, 30, 0, "Dirty Air");
  GD.Tag(31);  //label then button
  GD.cmd_button(x + xs * 2, y, w, h, 30, 0, "Fecheimer");
 // GD.Tag(32);  //label then button
 // GD.cmd_button(x + xs * 2, y + ys, w, h, 30, 0, "Empty");
 // GD.Tag(33);  //label then button
 // GD.cmd_button(x + xs * 2, y + ys * 2, w, h, 30, 0, "Empty");
  // if (minnow != minprev) {  //updates serial every minute
  //   minprev++;
  //   if (minprev == 60) {
  //     minprev -= 60;
  //   }
  //   Serial.println(timestampl);
  // }

  // Serial.println(GD.inputs.tag); //prints 0 when nothing pressed, prints 255 when open area pressed
  if (GD.inputs.tag == 2) {
    Serial.print("Button Pressed ");
    Serial.println(GD.inputs.tag);
    delay(200);
    page = 2;
    Serial.print("Page - ");
    Serial.println(page);
  }
  if (GD.inputs.tag == 11) {  //baro pressure
    Serial.print("Baro Page Button Pressed ");
    Serial.println(GD.inputs.tag);
    delay(200);  //secprev = now.second();
    secprev = secnow;
    page = 11;
    Serial.print("BP - Page ");
    Serial.println(page);
  }
  if (GD.inputs.tag == 12) {  //temp page
    Serial.print("Temp Page Button Pressed ");
    Serial.println(GD.inputs.tag);
    delay(200);
    page = 12;
    Serial.print("Page - ");
    Serial.println(page);
  }
  if (GD.inputs.tag == 13) {
    Serial.print("Button Pressed ");
    Serial.println(GD.inputs.tag);
    delay(200);
    myDP10.clear();
    myDP24.clear();
    myDP138.clear();
    myDP01.clear();
    myRADP10.clear();
    myRADP24.clear();
    myRADP138.clear();
    myRADP01.clear();
    DP10AVG = 0, DP24AVG = 0, DP138AVG = 0, DP01AVG = 0;
    page = 13;
    Serial.print("Page - ");
    Serial.println(page);
  }
  if (GD.inputs.tag == 21) {
    Serial.print("Button Pressed ");
    Serial.println(GD.inputs.tag);
    delay(200);
    myDP10.clear();
    myDP24.clear();
    myDP138.clear();
    myDP01.clear();
    myRADP10.clear();
    myRADP24.clear();
    myRADP138.clear();
    myRADP01.clear();
    DP10AVG = 0, DP24AVG = 0, DP138AVG = 0, DP01AVG = 0;
    page = 21;
    Serial.print("Page - ");
    Serial.println(page);
  }
  if (GD.inputs.tag == 23) {
    Serial.print("Button Pressed ");
    Serial.println(GD.inputs.tag);
    delay(200);
    myDP10.clear();
    myDP24.clear();
    myDP138.clear();
    myDP01.clear();
    myRADP10.clear();
    myRADP24.clear();
    myRADP138.clear();
    myRADP01.clear();
    DP10AVG = 0, DP24AVG = 0, DP138AVG = 0;
    DP01AVG = 0;
    page = 23;
    Serial.print("Page - ");
    Serial.println(page);
  }
  if (GD.inputs.tag == 31) {
    Serial.print("Button Pressed ");
    Serial.println(GD.inputs.tag);
    delay(200);
    myDP10.clear();
    myDP24.clear();
    myDP138.clear();
    myDP01.clear();
    myRADP10.clear();
    myRADP24.clear();
    myRADP138.clear();
    myRADP01.clear();
    DP10AVG = 0, DP24AVG = 0, DP138AVG = 0;
    DP01AVG = 0;
    SPAVG = 0, DPAVG = 0, NullAVG = 0, PitchAVG = 0;
    page = 31;
    Serial.print("Page - ");
    Serial.println(page);
  }
  GD.swap();
}

/*//////////////////Settings Page - Page 2///////////////////////////////////////////////////////////////////////*/
void Page2() {
  GD.ClearColorRGB(0, 0, 0);
  GD.Clear();
  header();
  GD.get_inputs();
  GD.cmd_text(GD.w / 2, 40, 29, OPT_CENTERX, "Settings");
  GD.Tag(1);
  GD.cmd_button(2, 2, 100, 50, 30, 0, "HOME");
  GD.cmd_text(400, 120, 30, 0, "Thermo Type");
  GD.Tag(13);
  GD.cmd_button(410, 160, 100, 50, 30, 0, "K");
  GD.Tag(14);
  GD.cmd_button(540, 160, 100, 50, 30, 0, "");
  GD.cmd_text(400, 220, 30, 0, "Record Time (sec)");
  GD.Tag(41);
  GD.cmd_button(410, 260, 100, 50, 30, 0, "2");
  GD.Tag(42);
  GD.cmd_button(540, 260, 100, 50, 30, 0, "5");
  GD.Tag(43);
  GD.cmd_button(670, 260, 100, 50, 30, 0, "10");
  GD.Tag(44);
  GD.cmd_button(410, 330, 100, 50, 30, 0, "30");
  GD.Tag(2);
  GD.cmd_button(500, 400, 150, 50, 30, 0, "Sensors");

  if (GD.inputs.tag == 1) {  //page 1 - home page
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    page = 1;
    Serial.print("Page - ");
    Serial.println(page);
  }
  if (GD.inputs.tag == 2) {  //Sensors Setting Page
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    page = 3;
    Serial.print("Page -");
    Serial.println(page);
  }
  if (GD.inputs.tag == 41) {  //Record Time
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(dprecadr, 2);
    secrec = EEPROM.readInt(dprecadr);
    rectime = secrec;
    Serial.print("Record time - EEPROM: ");
    Serial.println(EEPROM.readInt(dprecadr));
  }
  if (GD.inputs.tag == 42) {  //Record Time
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(dprecadr, 5);
    secrec = EEPROM.readInt(dprecadr);
    rectime = secrec;
    Serial.print("Record time - EEPROM: ");
    Serial.println(EEPROM.readInt(dprecadr));
  }
  if (GD.inputs.tag == 43) {  //Record Time
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(dprecadr, 10);
    secrec = EEPROM.readInt(dprecadr);
    rectime = secrec;
    Serial.print("Record time - EEPROM: ");
    Serial.println(EEPROM.readInt(dprecadr));
  }
  if (GD.inputs.tag == 44) {  //Record Time
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(dprecadr, 30);
    secrec = EEPROM.readInt(dprecadr);
    rectime = secrec;
    Serial.print("Record time - EEPROM: ");
    Serial.println(EEPROM.readInt(dprecadr));
  }
  //insert variable for thermocouple type
  GD.swap();
}

/*//////////////////Sensors Settings Page - Page 3///////////////////////////////////////////////////////////////////////*/
void Page3() {
  GD.ClearColorRGB(0, 0, 0);
  GD.Clear();
  header();
  GD.get_inputs();
  int x = 75, xs = 130, y = 100, ys = 80;

  GD.cmd_text(GD.w / 2, 40, 29, OPT_CENTERX, "Settings");
  GD.cmd_text(x - 35, y + 10, 29, OPT_CENTERX, "DP");
  GD.cmd_text(x - 35, y + ys + 10, 29, OPT_CENTERX, "SP");
  GD.cmd_text(x - 35, y + 2 * ys + 10, 29, OPT_CENTERX, "Null");
  GD.cmd_text(x - 35, y + 3 * ys + 10, 29, OPT_CENTERX, "Pitch");

  GD.cmd_text(x + 4 * xs + 150, y - 25, 27, OPT_CENTERX, "SELECTED");
  GD.cmd_number(x + 4 * xs + 150, y + 10, 29, OPT_CENTERX, dpsensor);
  GD.cmd_number(x + 4 * xs + 150, y + ys + 10, 29, OPT_CENTERX, spsensor);
  GD.cmd_number(x + 4 * xs + 150, y + ys * 2 + 10, 29, OPT_CENTERX, nullsensor);
  GD.cmd_number(x + 4 * xs + 150, y + ys * 3 + 10, 29, OPT_CENTERX, pitchsensor);

  GD.Tag(1);
  GD.cmd_button(2, 2, 110, 50, 30, 0, "HOME");
  //Tag 2-6 is for DP
  GD.Tag(2);
  GD.cmd_button(x, y, 110, 50, 30, 0, "+/- 1");
  GD.Tag(3);
  GD.cmd_button(x + xs, y, 110, 50, 30, 0, "+/- 5");
  GD.Tag(4);
  GD.cmd_button(x + 2 * xs, y, 110, 50, 30, 0, "+/- 10");
  GD.Tag(5);
  GD.cmd_button(x + 3 * xs, y, 110, 50, 30, 0, "+/-24");
  GD.Tag(6);
  GD.cmd_button(x + 4 * xs, y, 110, 50, 30, 0, "+/- 138");
  //Tag 7-11 is for SP
  GD.Tag(7);
  GD.cmd_button(x, y + ys, 110, 50, 30, 0, "+/- 1");
  GD.Tag(8);
  GD.cmd_button(x + xs, y + ys, 110, 50, 30, 0, "+/- 5");
  GD.Tag(9);
  GD.cmd_button(x + 2 * xs, y + ys, 110, 50, 30, 0, "+/- 10");
  GD.Tag(10);
  GD.cmd_button(x + 3 * xs, y + ys, 110, 50, 30, 0, "+/-24");
  GD.Tag(11);
  GD.cmd_button(x + 4 * xs, y + ys, 110, 50, 30, 0, "+/- 138");
  //Tag 12-16 is for Null
  GD.Tag(12);
  GD.cmd_button(x, y + 2 * ys, 110, 50, 30, 0, "+/- 1");
  GD.Tag(13);
  GD.cmd_button(x + xs, y + 2 * ys, 110, 50, 30, 0, "+/- 5");
  GD.Tag(14);
  GD.cmd_button(x + 2 * xs, y + 2 * ys, 110, 50, 30, 0, "+/- 10");
  GD.Tag(15);
  GD.cmd_button(x + 3 * xs, y + 2 * ys, 110, 50, 30, 0, "+/-24");
  GD.Tag(16);
  GD.cmd_button(x + 4 * xs, y + 2 * ys, 110, 50, 30, 0, "+/- 138");
  //Tag 17-21 is for Pitch
  GD.Tag(17);
  GD.cmd_button(x, y + 3 * ys, 110, 50, 30, 0, "+/- 1");
  GD.Tag(18);
  GD.cmd_button(x + xs, y + 3 * ys, 110, 50, 30, 0, "+/- 5");
  GD.Tag(19);
  GD.cmd_button(x + 2 * xs, y + 3 * ys, 110, 50, 30, 0, "+/- 10");
  GD.Tag(20);
  GD.cmd_button(x + 3 * xs, y + 3 * ys, 110, 50, 30, 0, "+/-24");
  GD.Tag(21);
  GD.cmd_button(x + 4 * xs, y + 3 * ys, 110, 50, 30, 0, "+/- 138");

  if (GD.inputs.tag == 1) {  //page 1 - home page
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    page = 1;
    Serial.print("Page - ");
    Serial.println(page);
  }
  if (GD.inputs.tag == 2) {  //DP Sensor Value to 1 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(dpsensoradr, 1);
    dpsensor = EEPROM.readInt(dpsensoradr);
    Serial.print("DP Sensor Set to 1 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(dpsensoradr));
  }
  if (GD.inputs.tag == 3) {  //DP Sensor Value to 5 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(dpsensoradr, 5);
    dpsensor = EEPROM.readInt(dpsensoradr);
    Serial.print("DP Sensor Set to 5 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(dpsensoradr));
  }
  if (GD.inputs.tag == 4) {  //DP Sensor Value to 10 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(dpsensoradr, 10);
    dpsensor = EEPROM.readInt(dpsensoradr);
    Serial.print("DP Sensor Set to 10 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(dpsensoradr));
  }
  if (GD.inputs.tag == 5) {  //DP Sensor Value to 24 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(dpsensoradr, 24);
    dpsensor = EEPROM.readInt(dpsensoradr);
    Serial.print("DP Sensor Set to 24 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(dpsensoradr));
  }
  if (GD.inputs.tag == 6) {  //DP Sensor Value to 138 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(dpsensoradr, 138);
    dpsensor = EEPROM.readInt(dpsensoradr);
    Serial.print("DP Sensor Set to 138 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(dpsensoradr));
  }
  if (GD.inputs.tag == 7) {  //SP Sensor Value to 1 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(spsensoradr, 1);
    spsensor = EEPROM.readInt(spsensoradr);
    Serial.print("SP Sensor Set to 1 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(spsensoradr));
  }
  if (GD.inputs.tag == 8) {  //SP Sensor Value to 5 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(spsensoradr, 5);
    spsensor = EEPROM.readInt(spsensoradr);
    Serial.print("SP Sensor Set to 5 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(spsensoradr));
  }
  if (GD.inputs.tag == 9) {  //SP Sensor Value to 10 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(spsensoradr, 10);
    spsensor = EEPROM.readInt(spsensoradr);
    Serial.print("SP Sensor Set to 10 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(spsensoradr));
  }
  if (GD.inputs.tag == 10) {  //SP Sensor Value to 24 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(spsensoradr, 24);
    spsensor = EEPROM.readInt(spsensoradr);
    Serial.print("SP Sensor Set to 24 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(spsensoradr));
  }
  if (GD.inputs.tag == 11) {  //SP Sensor Value to 138 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(spsensoradr, 138);
    spsensor = EEPROM.readInt(spsensoradr);
    Serial.print("SP Sensor Set to 138 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(spsensoradr));
  }
  if (GD.inputs.tag == 12) {  //Null Sensor Value to 1 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(nullsensoradr, 1);
    nullsensor = EEPROM.readInt(nullsensoradr);
    Serial.print("Null Sensor Set to 1 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(nullsensoradr));
  }
  if (GD.inputs.tag == 13) {  //Null Sensor Value to 5 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(nullsensoradr, 5);
    nullsensor = EEPROM.readInt(nullsensoradr);
    Serial.print("Null Sensor Set to 5 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(nullsensoradr));
  }
  if (GD.inputs.tag == 14) {  //Null Sensor Value to 10 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(nullsensoradr, 10);
    nullsensor = EEPROM.readInt(nullsensoradr);
    Serial.print("Null Sensor Set to 10 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(nullsensoradr));
  }
  if (GD.inputs.tag == 15) {  //Null Sensor Value to 24 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(nullsensoradr, 24);
    nullsensor = EEPROM.readInt(nullsensoradr);
    Serial.print("Null Sensor Set to 24 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(nullsensoradr));
  }
  if (GD.inputs.tag == 16) {  //Null Sensor Value to 138 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(nullsensoradr, 138);
    nullsensor = EEPROM.readInt(nullsensoradr);
    Serial.print("Null Sensor Set to 138 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(nullsensoradr));
  }
  if (GD.inputs.tag == 17) {  //Pitch Sensor Value to 1 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(pitchsensoradr, 1);
    pitchsensor = EEPROM.readInt(pitchsensoradr);
    Serial.print("Pitch Sensor Set to 1 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(pitchsensoradr));
  }
  if (GD.inputs.tag == 18) {  //Pitch Sensor Value to 5 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(pitchsensoradr, 5);
    pitchsensor = EEPROM.readInt(pitchsensoradr);
    Serial.print("Pitch Sensor Set to 5 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(pitchsensoradr));
  }
  if (GD.inputs.tag == 19) {  //Pitch Sensor Value to 10 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(pitchsensoradr, 10);
    pitchsensor = EEPROM.readInt(pitchsensoradr);
    Serial.print("Pitch Sensor Set to 10 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(pitchsensoradr));
  }
  if (GD.inputs.tag == 20) {  //Pitch Sensor Value to 24 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(pitchsensoradr, 24);
    pitchsensor = EEPROM.readInt(pitchsensoradr);
    Serial.print("Pitch Sensor Set to 24 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(pitchsensoradr));
  }
  if (GD.inputs.tag == 21) {  //Pitch Sensor Value to 138 inwc
    Serial.print("inputs");
    Serial.println(GD.inputs.tag);
    delay(200);
    EEPROM.updateInt(pitchsensoradr, 138);
    pitchsensor = EEPROM.readInt(pitchsensoradr);
    Serial.print("Pitch Sensor Set to 138 inwc - EEPROM: ");
    Serial.println(EEPROM.readInt(pitchsensoradr));
  }
  GD.swap();
}

/*//////////////////Barometric Page - Page 11///////////////////////////////////////////////////////////////////////*/
void Page11() {
  GD.ClearColorRGB(0x550000);
  GD.Clear();
  header();
  readbp();

  GD.get_inputs();
  GD.cmd_text(GD.w / 2, 40, 29, OPT_CENTERX, "Barometric Pressure");
  GD.cmd_text(GD.w / 2 + 60, GD.h / 2 + 10, 30, 0, "inhg");
  dtostrf(BPU, 5, 2, bpv);
  GD.cmd_text(GD.w / 2, GD.h / 2, 31, OPT_CENTERX, bpv);  //for display

  if (millis() > millisprev + (1000 * 5)) {  //update value on screen every 5 seconds
    millisprev = millis();
    BPU = myBP.getAverage();
    Serial.print(timestampl);
    Serial.print("\t Baro Pressure: ");
    Serial.print(BP, 3);
    Serial.println(" inHg");
  }

  GD.Tag(1);
  GD.cmd_button(2, 2, 100, 50, 30, 0, "HOME");

  if (GD.inputs.tag == 1) {  //page 1
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(200);
    page = 1;
    Serial.print("page - ");
    Serial.println(page);
  }
  GD.swap();
}

/*//////////////////Temp Page - Page 12///////////////////////////////////////////////////////////////////////*/
void Page12() {
  //Serial.println("Page 12 - Temperature ");
  GD.ClearColorRGB(0x550000);
  GD.Clear();
  header();
  readtp();
  GD.get_inputs();
  GD.cmd_text(GD.w / 2, 40, 29, OPT_CENTERX, "Temperature");

  GD.cmd_text(200, 100, 29, OPT_CENTERX, "Internal Temp");
  dtostrf(TPI1, 5, 1, tpv);
  GD.cmd_text(200, 150, 31, OPT_CENTERX, tpv);
  GD.cmd_text(665, 155, 30, 0, "F");  //for display_internal temp0
  // dtostrf(TPI2, 5, 1, tpv);
  // GD.cmd_text(200, 200, 31, OPT_CENTERX, tpv);
  // GD.cmd_text(665, 205, 30, 0, "F");  //for display_internal temp1

  GD.cmd_text(600, 100, 29, OPT_CENTERX, "External Temp");
  dtostrf(TPE1, 5, 1, tpv);
  GD.cmd_text(600, 150, 31, OPT_CENTERX, tpv);
  // GD.cmd_text(265, 155, 30, 0, "F");  //for display_external temp0
  // dtostrf(TPE2, 5, 1, tpv);
  // GD.cmd_text(600, 200, 31, OPT_CENTERX, tpv);
  // GD.cmd_text(265, 205, 30, 0, "F");  //for display_external temp1

  Serial.print("External Temp1: ");
  Serial.print(TPE1);
  Serial.print("  Internal Temp1: ");
  Serial.print(TPI1);
  // Serial.print("  External Temp2: ");
  // Serial.print(TPE2);
  // Serial.print("  Internal Temp2: ");
  // Serial.println(TPI2);

  GD.Tag(1);
  GD.cmd_button(2, 2, 100, 50, 30, 0, "HOME");

  if (GD.inputs.tag == 1) {  //page 1
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    page = 1;
    Serial.print("page - ");
    Serial.println(page);
  }
  GD.swap();
  delay(200);
}

/*//////////////////DP Page - Page 13///////////////////////////////////////////////////////////////////////*/
void Page13() {
  Serial.println();
  Serial.print("DP Page");
  GD.ClearColorRGB(0x550000);
  GD.Clear();
  header();
  readall();
  GD.get_inputs();
  GD.cmd_text(GD.w / 2, 40, 29, OPT_CENTERX, "Differential Pressure");
  DP10 = myDP10.getAverage();
  DP24 = myDP24.getAverage();
  DP138 = myDP138.getAverage();

  GD.cmd_text(2, 80, 31, 0, "DP +/-10 inwc");
  GD.cmd_text(200, 130, 31, 0, dp10v);
  dtostrf(DP10, 5, 2, dp10v);
  GD.cmd_text(310, 140, 30, 0, "inwc");  //for display
  GD.cmd_text(2, 200, 31, 0, "DP +/-24 inwc");
  GD.cmd_text(200, 250, 31, 0, dp24v);
  dtostrf(DP24, 5, 2, dp24v);
  GD.cmd_text(310, 260, 30, 0, "inwc");  //for display
  GD.cmd_text(2, 320, 31, 0, "DP +/-138 inwc");
  GD.cmd_text(200, 380, 31, 0, dp138v);
  dtostrf(DP138, 5, 2, dp138v);
  GD.cmd_text(310, 390, 30, 0, "inwc");  //for display
  GD.cmd_text(400, 80, 28, 0, "Record (sec)");
  GD.cmd_number(600, 80, 28, 0, rectime);

  dtostrf(DP10AVG, 5, 2, dp10a);
  GD.cmd_text(550, 130, 31, 0, dp10a);  //average values
  dtostrf(DP24AVG, 5, 2, dp24a);
  GD.cmd_text(550, 250, 31, 0, dp24a);  //average values
  dtostrf(DP138AVG, 5, 2, dp138a);
  GD.cmd_text(550, 380, 31, 0, dp138a);  //average values

  Serial.print("\tDP01: ");
  Serial.print(DP01, 3);
  Serial.print("inwc");
  Serial.print("\tDP10: ");
  Serial.print(DP10, 3);
  Serial.print("inwc");
  Serial.print("\tDP24: ");
  Serial.print(DP24, 3);
  Serial.print("inwc");
  Serial.print("\tDP138: ");
  Serial.print(DP138, 3);
  Serial.print("inwc");
  Serial.print("\tRecord: ");
  Serial.print(rec);

  GD.Tag(1);
  GD.cmd_button(2, 2, 100, 50, 30, 0, "HOME");
  GD.Tag(21);
  GD.cmd_button(400, 120, 100, 75, 30, 0, "ZERO");
  GD.Tag(22);
  GD.cmd_button(400, 240, 100, 75, 30, 0, "ZERO");
  GD.Tag(23);
  GD.cmd_button(400, 370, 100, 75, 30, 0, "ZERO");
  GD.Tag(11);
  GD.cmd_button(690, 120, 100, 75, 30, 0, "AVG");
  GD.Tag(12);
  GD.cmd_button(690, 240, 100, 75, 30, 0, "AVG");
  GD.Tag(13);
  GD.cmd_button(690, 370, 100, 75, 30, 0, "AVG");

  if (GD.inputs.tag == 1) {  //page 1
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    rec = 0;
    page = 1;
    Serial.print("page - ");
    Serial.println(page);
  } else if (GD.inputs.tag == 21) {  //Zero first row DP raw value
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    sensorzero = 1;
    dpzero();
  } else if (GD.inputs.tag == 22) {  //Zero second row DP raw value
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    sensorzero = 2;
    dpzero();
  } else if (GD.inputs.tag == 23) {  //Zero third row DP raw value
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    sensorzero = 3;
    dpzero();
  } else if (GD.inputs.tag == 11) {  //DP record 10inwc
    Serial.print(" Button: ");
    Serial.print(GD.inputs.tag);
    delay(145);
    rec = 1;
    dprec = 1;
    secprev = secnow;
    secelps = 0;
    myRADP10.clear();
  } else if (GD.inputs.tag == 12) {  //DP record 24inwc
    Serial.print(" Button: ");
    Serial.print(GD.inputs.tag);
    delay(145);
    rec = 1;
    dprec = 2;
    secprev = secnow;
    secelps = 0;
    myRADP24.clear();
  } else if (GD.inputs.tag == 13) {  //DP record 138inwc
    Serial.print(" Button: ");
    Serial.print(GD.inputs.tag);
    delay(145);
    rec = 1;
    dprec = 3;
    secprev = secnow;
    secelps = 0;
    myRADP138.clear();
  }
  if (rec == 1) {
    avg();
  }
  GD.swap();
}

/*//////////////////PA Page - Page 21///////////////////////////////////////////////////////////////////////*/
void Page21() {
  Serial.println();
  Serial.print("PA Page");
  GD.ClearColorRGB(0x550000);
  GD.Clear();
  header();
  readall();
  readtp();
  GD.get_inputs();
  GD.cmd_text(GD.w / 2, 40, 29, OPT_CENTERX, "PRIMARY AIR");
  DP24 = myDP24.getAverage();
  DP138 = myDP138.getAverage();

  GD.cmd_text(5, 60, 29, 0, "VH +/-24 inwc");
  dtostrf(DP24, 5, 2, dp24v);
  GD.cmd_text(29, 100, 29, 0, dp24v);  //displays raw DP value (24inwc)
  GD.cmd_text(5, 145, 29, 0, "SP +/- 138 inwc");
  dtostrf(DP138, 5, 1, dp138v);
  GD.cmd_text(29, 185, 29, 0, dp138v);  //displays raw SP value (138inwc)
  GD.cmd_text(5, 230, 29, 0, "Temp.");
  dtostrf(TPE1, 5, 0, tpv);
  GD.cmd_text(100, 230, 29, 0, tpv);
  GD.cmd_text(175, 230, 29, 0, "F");  //displays raw temp reading
  GD.cmd_number(160, 260, 28, 0, rectime);
  GD.cmd_text(15, 260, 28, 0, "RECORD (SEC)");  //displays record time
  GD.cmd_text(650, 260, 29, OPT_CENTERX, "POINT");
  GD.cmd_number(650, 360, 29, OPT_CENTERX, papoint);  //Displays Point #
  GD.cmd_text(725, 80, 29, 0, "Port");
  GD.cmd_number(775, 80, 29, 0, paport);
  // GD.cmd_text(655, 80, 29, 0, "Test"); GD.cmd_number(705, 80, 29, 0, patest);

  int x = 250, y = 125, ys = 33, font = 29, xa = x + 60, xa1 = xa + 70, xa2 = xa1 + 60;
  mypatest = patest - 1;
  mypaport = paport - 1;                             //Array index for display
  GD.cmd_text(xa, y - 40, font, OPT_CENTERX, "VH");  // the following six rows are for the DP average
  GD.cmd_text(x, y, font, OPT_CENTERX, "1)");
  dtostrf(PAVH[mypatest][mypaport][0], 5, 2, dp24a);
  GD.cmd_text(xa, y, font, OPT_CENTERX, dp24a);  //average DP values point 1
  GD.cmd_text(x, y + ys, font, OPT_CENTERX, "2)");
  dtostrf(PAVH[mypatest][mypaport][1], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys, font, OPT_CENTERX, dp24a);  //average DP values point 2
  GD.cmd_text(x, y + ys * 2, font, OPT_CENTERX, "3)");
  dtostrf(PAVH[mypatest][mypaport][2], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 2, font, OPT_CENTERX, dp24a);  //average DP values point 3
  GD.cmd_text(x, y + ys * 3, font, OPT_CENTERX, "4)");
  dtostrf(PAVH[mypatest][mypaport][3], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 3, font, OPT_CENTERX, dp24a);  //average DP values point 4
  GD.cmd_text(x, y + ys * 4, font, OPT_CENTERX, "5)");
  dtostrf(PAVH[mypatest][mypaport][4], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 4, font, OPT_CENTERX, dp24a);  //average DP values point 5
  GD.cmd_text(x, y + ys * 5, font, OPT_CENTERX, "6)");
  dtostrf(PAVH[mypatest][mypaport][5], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 5, font, OPT_CENTERX, dp24a);  //average DP values point 6
  GD.cmd_text(x, y + ys * 6, font, OPT_CENTERX, "7)");
  dtostrf(PAVH[mypatest][mypaport][6], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 6, font, OPT_CENTERX, dp24a);  //average DP values point 3
  GD.cmd_text(x, y + ys * 7, font, OPT_CENTERX, "8)");
  dtostrf(PAVH[mypatest][mypaport][7], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 7, font, OPT_CENTERX, dp24a);  //average DP values point 4
  GD.cmd_text(x, y + ys * 8, font, OPT_CENTERX, "9)");
  dtostrf(PAVH[mypatest][mypaport][8], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 8, font, OPT_CENTERX, dp24a);  //average DP values point 5
  GD.cmd_text(x, y + ys * 9, font, OPT_CENTERX, "10)");
  dtostrf(PAVH[mypatest][mypaport][9], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 9, font, OPT_CENTERX, dp24a);  //average DP values point 6

  GD.cmd_text(xa1, y - 40, font, OPT_CENTERX, "SP");  // the following six rows are for the SP average
  dtostrf(PASP[mypatest][mypaport][0], 5, 1, dp138a);
  GD.cmd_text(xa1, y, font, OPT_CENTERX, dp138a);  //average SP values point 1
  dtostrf(PASP[mypatest][mypaport][1], 5, 1, dp138a);
  GD.cmd_text(xa1, y + ys, font, OPT_CENTERX, dp138a);  //average SP values point 2
  dtostrf(PASP[mypatest][mypaport][2], 5, 1, dp138a);
  GD.cmd_text(xa1, y + ys * 2, font, OPT_CENTERX, dp138a);  //average SP values point 3
  dtostrf(PASP[mypatest][mypaport][3], 5, 1, dp138a);
  GD.cmd_text(xa1, y + ys * 3, font, OPT_CENTERX, dp138a);  //average SP values point 4
  dtostrf(PASP[mypatest][mypaport][4], 5, 1, dp138a);
  GD.cmd_text(xa1, y + ys * 4, font, OPT_CENTERX, dp138a);  //average SP values point 5
  dtostrf(PASP[mypatest][mypaport][5], 5, 1, dp138a);
  GD.cmd_text(xa1, y + ys * 5, font, OPT_CENTERX, dp138a);  //average SP values point 6
  dtostrf(PASP[mypatest][mypaport][6], 5, 1, dp138a);
  GD.cmd_text(xa1, y + ys * 6, font, OPT_CENTERX, dp138a);  //average SP values point 7
  dtostrf(PASP[mypatest][mypaport][7], 5, 1, dp138a);
  GD.cmd_text(xa1, y + ys * 7, font, OPT_CENTERX, dp138a);  //average SP values point 8
  dtostrf(PASP[mypatest][mypaport][8], 5, 1, dp138a);
  GD.cmd_text(xa1, y + ys * 8, font, OPT_CENTERX, dp138a);  //average SP values point 9
  dtostrf(PASP[mypatest][mypaport][9], 5, 1, dp138a);
  GD.cmd_text(xa1, y + ys * 9, font, OPT_CENTERX, dp138a);  //average SP values point 10

  GD.cmd_text(xa2 + 10, y - 40, font, OPT_CENTERX, "TEMP");  // the following six rows are for the TEMP average
  dtostrf(PATP[mypatest][mypaport][0], 5, 0, tpi);
  GD.cmd_text(xa2, y, font, OPT_CENTERX, tpi);  //average TEMP values point 1
  dtostrf(PATP[mypatest][mypaport][1], 5, 0, tpi);
  GD.cmd_text(xa2, y + ys, font, OPT_CENTERX, tpi);  //average TEMP values point 2
  dtostrf(PATP[mypatest][mypaport][2], 5, 0, tpi);
  GD.cmd_text(xa2, y + ys * 2, font, OPT_CENTERX, tpi);  //average TEMP values point 3
  dtostrf(PATP[mypatest][mypaport][3], 5, 0, tpi);
  GD.cmd_text(xa2, y + ys * 3, font, OPT_CENTERX, tpi);  //average TEMP values point 4
  dtostrf(PATP[mypatest][mypaport][4], 5, 0, tpi);
  GD.cmd_text(xa2, y + ys * 4, font, OPT_CENTERX, tpi);  //average TEMP values point 5
  dtostrf(PATP[mypatest][mypaport][5], 5, 0, tpi);
  GD.cmd_text(xa2, y + ys * 5, font, OPT_CENTERX, tpi);  //average TEMP values point 6
  dtostrf(PATP[mypatest][mypaport][6], 5, 0, tpi);
  GD.cmd_text(xa2, y + ys * 6, font, OPT_CENTERX, tpi);  //average TEMP values point 7
  dtostrf(PATP[mypatest][mypaport][7], 5, 0, tpi);
  GD.cmd_text(xa2, y + ys * 7, font, OPT_CENTERX, tpi);  //average TEMP values point 8
  dtostrf(PATP[mypatest][mypaport][8], 5, 0, tpi);
  GD.cmd_text(xa2, y + ys * 8, font, OPT_CENTERX, tpi);  //average TEMP values point 9
  dtostrf(PATP[mypatest][mypaport][9], 5, 0, tpi);
  GD.cmd_text(xa2, y + ys * 9, font, OPT_CENTERX, tpi);  //average TEMP values point 10

  currentMillis = millis();  //get the current time
  elpsMillis = (currentMillis - lastMillis);
  seconds = (currentMillis - startMillis) / 1000;
  lastMillis = currentMillis;

  // Serial.print("\tDP10: "); Serial.print(DP10, 3); Serial.print("inwc");
  Serial.print("\tDP24: ");
  Serial.print(DP24, 3);
  Serial.print("inwc");
  Serial.print("\tDP138: ");
  Serial.print(DP138, 3);
  Serial.print("inwc");
  Serial.print("\tTemp: ");
  Serial.print(TPE1, 3);
  Serial.print("F");
  Serial.print("\tRecord: ");
  Serial.print(rec);

  GD.Tag(1);
  GD.cmd_button(10, 10, 100, 50, 30, 0, "HOME");
  GD.Tag(2);
  GD.cmd_button(110, 90, 85, 50, 29, 0, "ZERO");  //Zeros 24inwc DP
  GD.Tag(3);
  GD.cmd_button(110, 175, 85, 50, 29, 0, "ZERO");  //Zero 138inwc SP
  GD.Tag(4);
  GD.cmd_button(705, 250, 85, 50, 30, OPT_CENTERX, "VH");  //average 24inwc dp
  GD.Tag(5);
  GD.cmd_button(705, 310, 85, 50, 30, OPT_CENTERX, "SP");  //average 138inwc dp
  GD.Tag(6);
  GD.cmd_button(705, 370, 85, 50, 30, OPT_CENTERX, "TEMP");  //average TEMP
  GD.Tag(7);
  GD.cmd_button(705, 430, 85, 50, 30, OPT_CENTERX, "ALL");  //averages 138, 24, and temperature
  GD.Tag(8);
  GD.cmd_button(620, 300, 65, 50, 29, 0, " - ");  //Toggle Left (Decreases Point #)
  GD.Tag(9);
  GD.cmd_button(620, 400, 65, 50, 29, 0, " + ");  //Toggle Right (Increases Point #)
  //GD.Tag(10); GD.cmd_button(15,290,185,50,29,0,"CLEAR Test");
  GD.Tag(11);
  GD.cmd_button(15, 360, 185, 50, 29, 0, "CLEAR PORT");
  GD.Tag(12);
  GD.cmd_button(15, 430, 185, 50, 29, 0, "CLEAR ALL");
  GD.Tag(13);
  GD.cmd_button(725, 110, 65, 50, 29, 0, "-");  //Decreases Port #
  GD.Tag(14);
  GD.cmd_button(725, 170, 65, 50, 29, 0, "+");  //Increases Port #
  //GD.Tag(15); GD.cmd_button(650, 110, 65, 50, 29, 0,"-");//Decreases Test #
  //GD.Tag(16); GD.cmd_button(650, 170, 65, 50, 29, 0,"+");//Increases Test #


  if (GD.inputs.tag == 1) {  //Home Page
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    rec = 0;
    page = 1;
    Serial.print("page - ");
    Serial.println(page);
  } else if (GD.inputs.tag == 2) {  //zero sensor 24inwc DP
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    sensorzero = 2;
    dpzero();
  } else if (GD.inputs.tag == 3) {  //zero sensor 138inwc SP
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    sensorzero = 3;
    dpzero();
  } else if (GD.inputs.tag == 4) {  //DP record 24inwc
    Serial.print(" Button: ");
    Serial.print(GD.inputs.tag);
    delay(145);
    rec = 1;
    dprec = 2;
    DateTime now = rtc.now();
    secprev = now.second();
    secelps = 0;
    myRADP24.clear();
  } else if (GD.inputs.tag == 5) {  //DP record 138inwc
    Serial.print(" Button: ");
    Serial.print(GD.inputs.tag);
    delay(145);
    rec = 1;
    dprec = 3;
    DateTime now = rtc.now();
    secprev = now.second();
    secelps = 0;
    myRADP138.clear();
  } else if (GD.inputs.tag == 6) {  //Temp record
    Serial.print(" Button: ");
    Serial.print(GD.inputs.tag);
    delay(145);
    rec = 1;
    dprec = 4;
    DateTime now = rtc.now();
    secprev = now.second();
    secelps = 0;
    myRATP.clear();
  } else if (GD.inputs.tag == 7) {  //All record (sp 138, dp 24, & temp)
    Serial.print(" Button: ");
    Serial.print(GD.inputs.tag);
    delay(145);
    rec = 1;
    dprec = 5;
    DateTime now = rtc.now();
    secprev = now.second();
    secelps = 0;
    myRATP.clear();
    myRADP10.clear();
    myRADP24.clear();
    myRADP138.clear();
  } else if (GD.inputs.tag == 8) {  //Decreases PA Point Number
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    if (papoint > 1) {
      papoint = papoint - 1;
    } else if (papoint <= 1) {
      papoint = 10;
    }
  } else if (GD.inputs.tag == 9) {  //Increases Point Number
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    if (papoint < 10) {
      papoint = papoint + 1;
    } else if (papoint == 10) {
      papoint = 1;
    }
  } else if (GD.inputs.tag == 11) {  //Clear Port
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    for (int j = 0; j < 10; j++) {
      PAVH[mypatest][mypaport][j] = 0;
      PASP[mypatest][mypaport][j] = 0;
      PATP[mypatest][mypaport][j] = 0;
    }
  } else if (GD.inputs.tag == 12) {  //Clear All
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    for (int i = 0; i < paport; i++) {
      for (int j = 0; j < 10; j++) {
        PAVH[mypatest][i][j] = 0;
        PASP[mypatest][i][j] = 0;
        PATP[mypatest][i][j] = 0;
      }
    }
  } else if (GD.inputs.tag == 13) {  //Decreases PA Port Number
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    if (paport > 1) {
      paport = paport - 1;
    } else if (paport <= 1) {
      paport = PAports;
    }
  } else if (GD.inputs.tag == 14) {  //Increases Port Number
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    if (paport < PAports) {
      paport = paport + 1;
    } else if (paport == PAports) {
      paport = 1;
    }
  }
  if (rec == 1) {
    avg();
  }
  if (papoint == 1) {  //following if statements move the cursor when point number is active
    GD.cmd_text(x - 30, y - 5, 29, 0, ">");
  }
  if (papoint == 2 || papoint == 3 || papoint == 4 || papoint == 5 || papoint == 6 || papoint == 7 || papoint == 8 || papoint == 9 || papoint == 10) {
    GD.cmd_text(x - 30, y - 5 + ys * (papoint - 1), 29, 0, ">");
  }
  GD.swap();
}

/*//////////////////Dirty Air Page - Page 23///////////////////////////////////////////////////////////////////////*/
void Page23() {
  Serial.println();
  Serial.print("DA Page");
  GD.ClearColorRGB(0x550000);
  GD.Clear();
  header();
  GD.get_inputs();
  readall();
  DP24 = myDP24.getAverage();

  GD.cmd_text(GD.w / 2, 40, 29, OPT_CENTERX, "DIRTY AIR PAGE");
  GD.cmd_text(2, 80, 30, 0, "Dirty Air DP +/-24 inwc");
  dtostrf(DP24, 5, 2, dp24v);
  GD.cmd_text(30, 130, 30, 0, dp24v);
  GD.cmd_text(110, 130, 30, 0, "inwc");  //displays raw DP value
  GD.cmd_number(175, 200, 28, 0, rectime);
  GD.cmd_text(30, 200, 28, 0, "RECORD (SEC)");  //displays record time
  GD.cmd_text(590, 140, 31, 0, "POINT");
  GD.cmd_number(730, 140, 30, 0, dapoint);  //Displays Point #
  GD.cmd_number(327, 245, 30, 0, dapipe);
  GD.cmd_text(295, 190, 31, 0, "PIPE");  //Display Pipe #
  GD.cmd_number(327, 390, 30, 0, daport);
  GD.cmd_text(285, 335, 31, 0, "PORT");  //Display Port #

  int x = 500, y = 73, ys = 33, font = 29, xa = x + 50;
  mydapipe = dapipe - 1;
  mydaport = daport - 1;  //Array index for display
  GD.cmd_text(x, y, 29, OPT_CENTERX, "1)");
  dtostrf(DA[mydapipe][mydaport][0], 5, 2, dp24a);
  GD.cmd_text(xa, y, 29, OPT_CENTERX, dp24a);  //average values point 1
  GD.cmd_text(x, y + ys, 29, OPT_CENTERX, "2)");
  dtostrf(DA[mydapipe][mydaport][1], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys, 29, OPT_CENTERX, dp24a);  //average values point 2
  GD.cmd_text(x, y + 2 * ys, 29, OPT_CENTERX, "3)");
  dtostrf(DA[mydapipe][mydaport][2], 5, 2, dp24a);
  GD.cmd_text(xa, y + 2 * ys, 29, OPT_CENTERX, dp24a);  //average values point 3
  GD.cmd_text(x, y + 3 * ys, 29, OPT_CENTERX, "4)");
  dtostrf(DA[mydapipe][mydaport][3], 5, 2, dp24a);
  GD.cmd_text(xa, y + 3 * ys, 29, OPT_CENTERX, dp24a);  //average values point 4
  GD.cmd_text(x, y + 4 * ys, 29, OPT_CENTERX, "5)");
  dtostrf(DA[mydapipe][mydaport][4], 5, 2, dp24a);
  GD.cmd_text(xa, y + 4 * ys, 29, OPT_CENTERX, dp24a);  //average values point 5
  GD.cmd_text(x, y + 5 * ys, 29, OPT_CENTERX, "6)");
  dtostrf(DA[mydapipe][mydaport][5], 5, 2, dp24a);
  GD.cmd_text(xa, y + 5 * ys, 29, OPT_CENTERX, dp24a);  //average values point 6
  GD.cmd_text(x, y + 6 * ys, 29, OPT_CENTERX, "7)");
  dtostrf(DA[mydapipe][mydaport][6], 5, 2, dp24a);
  GD.cmd_text(xa, y + 6 * ys, 29, OPT_CENTERX, dp24a);  //average values point 7
  GD.cmd_text(x, y + 7 * ys, 29, OPT_CENTERX, "8)");
  dtostrf(DA[mydapipe][mydaport][7], 5, 2, dp24a);
  GD.cmd_text(xa, y + 7 * ys, 29, OPT_CENTERX, dp24a);  //average values point 8
  GD.cmd_text(x, y + 8 * ys, 29, OPT_CENTERX, "9)");
  dtostrf(DA[mydapipe][mydaport][8], 5, 2, dp24a);
  GD.cmd_text(xa, y + 8 * ys, 29, OPT_CENTERX, dp24a);  //average values point 9
  GD.cmd_text(x, y + 9 * ys, 29, OPT_CENTERX, "10)");
  dtostrf(DA[mydapipe][mydaport][9], 5, 2, dp24a);
  GD.cmd_text(xa, y + 9 * ys, 29, OPT_CENTERX, dp24a);  //average values point 10
  GD.cmd_text(x, y + 10 * ys, 29, OPT_CENTERX, "11)");
  dtostrf(DA[mydapipe][mydaport][10], 5, 2, dp24a);
  GD.cmd_text(xa, y + 10 * ys, 29, OPT_CENTERX, dp24a);  //average values point 11
  GD.cmd_text(x, y + 11 * ys, 29, OPT_CENTERX, "12)");
  dtostrf(DA[mydapipe][mydaport][11], 5, 2, dp24a);
  GD.cmd_text(xa, y + 11 * ys, 29, OPT_CENTERX, dp24a);  //average values point 12

  GD.Tag(1);
  GD.cmd_button(20, 20, 100, 50, 30, 0, "HOME");
  GD.Tag(21);
  GD.cmd_button(200, 120, 100, 50, 30, 0, "ZERO");
  GD.Tag(2);
  GD.cmd_button(710, 80, 65, 50, 30, 0, " - ");  //Toggle Left (Decreases Point #)
  GD.Tag(3);
  GD.cmd_button(710, 195, 65, 50, 30, 0, " + ");  //Toggle Right (Increases Point #)
  GD.Tag(4);
  GD.cmd_button(625, 275, 165, 80, 30, 0, "AVERAGE");
  GD.Tag(5);
  GD.cmd_button(20, 250, 185, 50, 30, 0, "CLEAR ALL");
  GD.Tag(6);
  GD.cmd_button(240, 385, 65, 50, 30, 0, "<");  //Toggle Left (Decreases Port #)
  GD.Tag(7);
  GD.cmd_button(375, 385, 65, 50, 30, 0, ">");  //Toggle Right (Increases Port #)
  GD.Tag(8);
  GD.cmd_button(20, 400, 185, 50, 30, 0, "CLEAR PORT");
  GD.Tag(233);
  GD.cmd_button(625, 375, 165, 75, 30, 0, "SUMMARY");
  GD.Tag(9);
  GD.cmd_button(240, 240, 65, 50, 30, 0, "<");  //Decreases Pipe #
  GD.Tag(10);
  GD.cmd_button(375, 240, 65, 50, 30, 0, ">");  //Increases Pipe #
  GD.Tag(11);
  GD.cmd_button(20, 325, 185, 50, 30, 0, "CLEAR PIPE");

  Serial.print("\tDP24: ");
  Serial.print(DP24, 3);
  Serial.print("inwc");
  Serial.print("\tRecord: ");
  Serial.print(rec);
  Serial.print("\tDAPort: ");
  Serial.print(daport);
  Serial.print("\tdapoint: ");
  Serial.print(dapoint);
  Serial.print("\tDAPipe: ");
  Serial.print(dapipe);
  Serial.print("\tmyDApipe: ");
  Serial.print(mydapipe);
  Serial.print("\tmyDAport: ");
  Serial.print(mydaport);

  if (minnow != minprev) {  //updates serial every minute
    minprev++;
    if (minprev == 60) {
      minprev -= 60;
    }
    Serial.println(timestampl);
  }
  // Serial.println(GD.inputs.tag); //prints 0 when nothing pressed, prints 255 when open area pressed
  if (GD.inputs.tag == 1) {  //home
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    rec = 0;
    page = 1;
    Serial.print("page - ");
    Serial.println(page);
  } else if (GD.inputs.tag == 2) {  //Decreases Point Number
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    if (dapoint > 1) {
      dapoint = dapoint - 1;
    } else if (dapoint <= 1) {
      dapoint = 12;
    }
  } else if (GD.inputs.tag == 3) {  //Increases Point Number
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    if (dapoint < 12) {
      dapoint = dapoint + 1;
    } else if (dapoint == 12) {
      dapoint = 1;
    }
  } else if (GD.inputs.tag == 21) {  //zero 24inwc
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    sensorzero = 2;
    dpzero();
  } else if (GD.inputs.tag == 4) {  //Average Button
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    rec = 1;
    dprec = 2;
    secprev = secnow;
    secelps = 0;
    myRADP24.clear();
  } else if (GD.inputs.tag == 5) {  //clear all
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    memset(DA, 0, sizeof(DA));
  } else if (GD.inputs.tag == 8) {  //Clear Port
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    for (int j = 0; j < 12; j++) {
      DA[mydapipe][mydaport][j] = 0;
    }
  } else if (GD.inputs.tag == 11) {  //clears pipe
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    for (int i = 0; i < DAports; i++) {
      for (int j = 0; j < 12; j++)
        DA[mydapipe][i][j] = 0;
    }
  } else if (GD.inputs.tag == 6) {  //Decreases Port #
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    if (daport > 1) {
      daport = daport - 1;
    } else if (daport <= 1) {
      daport = DAports;
    }
  } else if (GD.inputs.tag == 7) {  //Increases Port #
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    if (daport < DAports) {
      daport = daport + 1;
    } else if (daport == DAports) {
      daport = 1;
    }
  } else if (GD.inputs.tag == 9) {  //Decreases Pipe #
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    if (dapipe > 1) {
      dapipe = dapipe - 1;
    } else if (dapipe <= 1) {
      dapipe = DApipes;
    }
  } else if (GD.inputs.tag == 10) {  //Increases Pipe #
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    if (dapipe < DApipes) {
      dapipe = dapipe + 1;
    } else if (dapipe == DApipes) {
      dapipe = 1;
    }
  }
  if (GD.inputs.tag == 233) {  //DA Summary Page
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    page = 233;
    Serial.print("page - ");
    Serial.println(page);
  }
  if (dapoint == 1) {  //following if statements move the cursor when point number is active
    GD.cmd_text(x - 30, y - 5, 29, 0, ">");
  }
  if (dapoint == 2 || dapoint == 3 || dapoint == 4 || dapoint == 5 || dapoint == 6 || dapoint == 7 || dapoint == 8 || dapoint == 9 || dapoint == 10 || dapoint == 11 || dapoint == 12) {
    GD.cmd_text(x - 30, y - 5 + ys * (dapoint - 1), 29, 0, ">");
  }
  if (rec == 1) {
    avg();
  }
  GD.swap();
}

/*//////////////////DA Summary Page///////////////////////////////////////////////////////////////////////*/
void Page233() {
  Serial.println();
  Serial.print("DA Summary Page");
  GD.ClearColorRGB(0x550000);
  GD.Clear();
  header();
  GD.get_inputs();
  GD.cmd_text(GD.w / 2, 40, 29, OPT_CENTERX, "DIRTY AIR SUMMARY PAGE");
  GD.cmd_number(87, 195, 30, 0, dapipe);
  GD.cmd_text(65, 160, 30, 0, "PIPE");
  int x = 200, y = 70, ys = 33, font1 = 30, dx = x - 15, px = x - 35, xa = x + 25, font2 = 30, x1 = x + 150, dx1 = x1 - 15, px1 = x1 - 35, xa1 = x1 + 25, x2 = x1 + 150, dx2 = x2 - 15, px2 = x2 - 35, xa2 = x2 + 25;  //each function is built off of "x". "x" is the initial position for port 1, the spacing was determined through trial/error

  mydapipe = dapipe - 1;
  GD.cmd_text(px, 238, 31, 0, "1");  //Displays port number
  GD.cmd_text(x, y, font1, 0, "1)");
  dtostrf(DA[mydapipe][0][0], 5, 2, dp24a);
  GD.cmd_text(xa, y, font2, 0, dp24a);  //average values point 1
  GD.cmd_text(x, y + ys, font1, 0, "2)");
  dtostrf(DA[mydapipe][0][1], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys, font2, 0, dp24a);  //average values point 2
  GD.cmd_text(x, y + ys * 2, font1, 0, "3)");
  dtostrf(DA[mydapipe][0][2], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 2, font2, 0, dp24a);  //average values point 3
  GD.cmd_text(x, y + ys * 3, font1, 0, "4)");
  dtostrf(DA[mydapipe][0][3], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 3, font2, 0, dp24a);  //average values point 4
  GD.cmd_text(x, y + ys * 4, font1, 0, "5)");
  dtostrf(DA[mydapipe][0][4], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 4, font2, 0, dp24a);  //average values point 5
  GD.cmd_text(x, y + ys * 5, font1, 0, "6)");
  dtostrf(DA[mydapipe][0][5], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 5, font2, 0, dp24a);  //average values point 6
  GD.cmd_text(x, y + ys * 6, font1, 0, "7)");
  dtostrf(DA[mydapipe][0][6], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 6, font2, 0, dp24a);  //average values point 7
  GD.cmd_text(x, y + ys * 7, font1, 0, "8)");
  dtostrf(DA[mydapipe][0][7], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 7, font2, 0, dp24a);  //average values point 8
  GD.cmd_text(x, y + ys * 8, font1, 0, "9)");
  dtostrf(DA[mydapipe][0][8], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 8, font2, 0, dp24a);  //average values point 9
  GD.cmd_text(dx, y + ys * 9, font1, 0, "10)");
  dtostrf(DA[mydapipe][0][9], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 9, font2, 0, dp24a);  //average values point 10
  GD.cmd_text(dx, y + ys * 10, font1, 0, "11)");
  dtostrf(DA[mydapipe][0][10], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 10, font2, 0, dp24a);  //average values point 11
  GD.cmd_text(dx, y + ys * 11, font1, 0, "12)");
  dtostrf(DA[mydapipe][0][11], 5, 2, dp24a);
  GD.cmd_text(xa, y + ys * 11, font2, 0, dp24a);  //average values point 12

  GD.cmd_text(px1, 238, 31, 0, "2");  //Displays port number
  GD.cmd_text(x1, y, font1, 0, "1)");
  dtostrf(DA[mydapipe][1][0], 5, 2, dp24a);
  GD.cmd_text(xa1, y, font2, 0, dp24a);  //average values point 1
  GD.cmd_text(x1, y + ys, font1, 0, "2)");
  dtostrf(DA[mydapipe][1][1], 5, 2, dp24a);
  GD.cmd_text(xa1, y + ys, font2, 0, dp24a);  //average values point 2
  GD.cmd_text(x1, y + ys * 2, font1, 0, "3)");
  dtostrf(DA[mydapipe][1][2], 5, 2, dp24a);
  GD.cmd_text(xa1, y + ys * 2, font2, 0, dp24a);  //average values point 3
  GD.cmd_text(x1, y + ys * 3, font1, 0, "4)");
  dtostrf(DA[mydapipe][1][3], 5, 2, dp24a);
  GD.cmd_text(xa1, y + ys * 3, font2, 0, dp24a);  //average values point 4
  GD.cmd_text(x1, y + ys * 4, font1, 0, "5)");
  dtostrf(DA[mydapipe][1][4], 5, 2, dp24a);
  GD.cmd_text(xa1, y + ys * 4, font2, 0, dp24a);  //average values point 5
  GD.cmd_text(x1, y + ys * 5, font1, 0, "6)");
  dtostrf(DA[mydapipe][1][5], 5, 2, dp24a);
  GD.cmd_text(xa1, y + ys * 5, font2, 0, dp24a);  //average values point 6
  GD.cmd_text(x1, y + ys * 6, font1, 0, "7)");
  dtostrf(DA[mydapipe][1][6], 5, 2, dp24a);
  GD.cmd_text(xa1, y + ys * 6, font2, 0, dp24a);  //average values point 7
  GD.cmd_text(x1, y + ys * 7, font1, 0, "8)");
  dtostrf(DA[mydapipe][1][7], 5, 2, dp24a);
  GD.cmd_text(xa1, y + ys * 7, font2, 0, dp24a);  //average values point 8
  GD.cmd_text(x1, y + ys * 8, font1, 0, "9)");
  dtostrf(DA[mydapipe][1][8], 5, 2, dp24a);
  GD.cmd_text(xa1, y + ys * 8, font2, 0, dp24a);  //average values point 9
  GD.cmd_text(dx1, y + ys * 9, font1, 0, "10)");
  dtostrf(DA[mydapipe][1][9], 5, 2, dp24a);
  GD.cmd_text(xa1, y + ys * 9, font2, 0, dp24a);  //average values point 10
  GD.cmd_text(dx1, y + ys * 10, font1, 0, "11)");
  dtostrf(DA[mydapipe][1][10], 5, 2, dp24a);
  GD.cmd_text(xa1, y + ys * 10, font2, 0, dp24a);  //average values point 11
  GD.cmd_text(dx1, y + ys * 11, font1, 0, "12)");
  dtostrf(DA[mydapipe][1][11], 5, 2, dp24a);
  GD.cmd_text(xa1, y + ys * 11, font2, 0, dp24a);  //average values point 12

  GD.cmd_text(px2, 238, 31, 0, "3");  //Displays port number
  GD.cmd_text(x2, y, font1, 0, "1)");
  dtostrf(DA[mydapipe][2][0], 5, 2, dp24a);
  GD.cmd_text(xa2, y, font2, 0, dp24a);  //average values point 1
  GD.cmd_text(x2, y + ys, font1, 0, "2)");
  dtostrf(DA[mydapipe][2][1], 5, 2, dp24a);
  GD.cmd_text(xa2, y + ys, font2, 0, dp24a);  //average values point 2
  GD.cmd_text(x2, y + ys * 2, font1, 0, "3)");
  dtostrf(DA[mydapipe][2][2], 5, 2, dp24a);
  GD.cmd_text(xa2, y + ys * 2, font2, 0, dp24a);  //average values point 3
  GD.cmd_text(x2, y + ys * 3, font1, 0, "4)");
  dtostrf(DA[mydapipe][2][3], 5, 2, dp24a);
  GD.cmd_text(xa2, y + ys * 3, font2, 0, dp24a);  //average values point 4
  GD.cmd_text(x2, y + ys * 4, font1, 0, "5)");
  dtostrf(DA[mydapipe][2][4], 5, 2, dp24a);
  GD.cmd_text(xa2, y + ys * 4, font2, 0, dp24a);  //average values point 5
  GD.cmd_text(x2, y + ys * 5, font1, 0, "6)");
  dtostrf(DA[mydapipe][2][5], 5, 2, dp24a);
  GD.cmd_text(xa2, y + ys * 5, font2, 0, dp24a);  //average values point 6
  GD.cmd_text(x2, y + ys * 6, font1, 0, "7)");
  dtostrf(DA[mydapipe][2][6], 5, 2, dp24a);
  GD.cmd_text(xa2, y + ys * 6, font2, 0, dp24a);  //average values point 7
  GD.cmd_text(x2, y + ys * 7, font1, 0, "8)");
  dtostrf(DA[mydapipe][2][7], 5, 2, dp24a);
  GD.cmd_text(xa2, y + ys * 7, font2, 0, dp24a);  //average values point 8
  GD.cmd_text(x2, y + ys * 8, font1, 0, "9)");
  dtostrf(DA[mydapipe][2][9], 5, 2, dp24a);
  GD.cmd_text(xa2, y + ys * 8, font2, 0, dp24a);  //average values point 9
  GD.cmd_text(dx2, y + ys * 9, font1, 0, "10)");
  dtostrf(DA[mydapipe][2][9], 5, 2, dp24a);
  GD.cmd_text(xa2, y + ys * 9, font2, 0, dp24a);  //average values point 10
  GD.cmd_text(dx2, y + ys * 10, font1, 0, "11)");
  dtostrf(DA[mydapipe][2][10], 5, 2, dp24a);
  GD.cmd_text(xa2, y + ys * 10, font2, 0, dp24a);  //average values point 11
  GD.cmd_text(dx2, y + ys * 11, font1, 0, "12)");
  dtostrf(DA[mydapipe][2][11], 5, 2, dp24a);
  GD.cmd_text(xa2, y + ys * 11, font2, 0, dp24a);  //average values point 12

  GD.Tag(23);
  GD.cmd_button(15, 15, 100, 50, 30, 0, "DA");  //DA Home Page
  GD.Tag(1);
  GD.cmd_button(15, 95, 75, 50, 30, 0, "<");  //Decreases Pipe #
  GD.Tag(2);
  GD.cmd_button(105, 95, 75, 50, 30, 0, ">");  //Increases Pipe #

  if (GD.inputs.tag == 23) {  //DA Page
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    rec = 0;
    page = 23;
    Serial.print("page - ");
    Serial.println(page);
  } else if (GD.inputs.tag == 1) {  //Decreases Pipe #
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    if (dapipe > 1) {
      dapipe = dapipe - 1;
    } else if (dapipe <= 1) {
      dapipe = DApipes;
    }
  } else if (GD.inputs.tag == 2) {  //Increases Pipe #
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    if (dapipe < DApipes) {
      dapipe = dapipe + 1;
    } else if (dapipe == DApipes) {
      dapipe = 1;
    }
  }
  GD.swap();
}

/*//////////////////Fecheimer - Page 31///////////////////////////////////////////////////////////////////////*/
void Page31() {
  // Serial.println();
  GD.ClearColorRGB(0x550000);
  GD.Clear();
  header();
  GD.get_inputs();
  readall();
  readtp();

  GD.cmd_text(GD.w / 2, 40, 29, OPT_CENTERX, "FECHEIMER PAGE");  //Header Info
  DP01 = myDP01.getAverage();
  DP05 = myDP05.getAverage();
  DP10 = myDP10.getAverage();
  DP24 = myDP24.getAverage();
  DP138 = myDP138.getAverage();

  GD.cmd_text(5, 70, 30, 0, "DP +/-");
  GD.cmd_number(5 + 100, 70, 30, 0, dpsensor);
  dtostrf(DP, 5, 2, dp24v);
  GD.cmd_text(5 + 24, 70 + 40, 30, 0, dp24v);  //displays raw DP value
  GD.cmd_text(5, 70 + 105, 30, 0, "SP +/-");
  GD.cmd_number(5 + 100, 70 + 105, 30, 0, spsensor);
  dtostrf(StatP, 5, 1, dp138v);
  GD.cmd_text(5 + 24, 70 + 105 + 40, 30, 0, dp138v);  //displays raw SP value
  GD.cmd_text(5, 70 + 105 * 2, 30, 0, "Null +/-");
  GD.cmd_number(5 + 100 + 10, 70 + 105 * 2, 30, 0, nullsensor);
  dtostrf(Null, 5, 2, dp01v);
  GD.cmd_text(5 + 24, 70 + 105 * 2 + 40, 30, 0, dp01v);  //displays raw Null value
  GD.cmd_text(5, 70 + 105 * 3, 30, 0, "Pitch +/-");
  GD.cmd_number(5 + 100 + 25, 70 + 105 * 3, 30, 0, pitchsensor);
  dtostrf(Pitch1, 5, 2, dp10v);
  GD.cmd_text(5 + 24, 70 + 105 * 3 + 40, 30, 0, dp10v);  //displays raw Pitch value

  GD.cmd_text(500, 70 + 50, 30, 0, "Temp.");
  GD.cmd_text(500 + 170, 70 + 50, 30, 0, "F");
  dtostrf(TPE1, 5, 0, tpv);
  GD.cmd_text(500 + 90, 70 + 50, 30, 0, tpv);
  GD.cmd_text(505, 70, 29, 0, "RECORD (SEC)");
  GD.cmd_number(505 + 175, 70, 29, 0, rectime);  //displays record time

  dtostrf(DPAVG, 5, 2, dpa);
  GD.cmd_text(5 + 24 + 220, 70 + 40, 30, 0, dpa);  //average value DP
  dtostrf(SPAVG, 5, 1, spa);
  GD.cmd_text(5 + 24 + 220, 70 + 40 + 105, 30, 0, spa);  //average value SP
  dtostrf(NullAVG, 5, 2, nulla);
  GD.cmd_text(5 + 24 + 220, 70 + 40 + 105 * 2, 30, 0, nulla);  //average value Null
  dtostrf(PitchAVG, 5, 2, pitcha);
  GD.cmd_text(5 + 24 + 220, 70 + 40 + 105 * 3, 30, 0, pitcha);  //average value Pitch


  switch (dpsensor) {  //changes dp raw sensor out, zero, and average functions when DP sensor is changed
    case 1:            //changes dp raw sensor out, zero, and average functions when DP sensor is 0-1 inwc
      DP = DP01;
      DPAVG = DP01AVG;
      if (GD.inputs.tag == 6) {  //Average DP01 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 6;
        secprev = secnow;
        secelps = 0;
        myRADP01.clear();
      } else if (GD.inputs.tag == 2) {  //zero DP01 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 4;
        dpzero();
      }
      break;

    case 5:  //changes dp raw sensor out, zero, and average functions when DP sensor is 0-5 inwc
      DP = DP05;
      DPAVG = DP05AVG;
      if (GD.inputs.tag == 6) {  //Average DP05 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 7;
        secprev = secnow;
        secelps = 0;
        myRADP05.clear();
      }
      if (GD.inputs.tag == 2) {  //zero DP05 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 5;
        dpzero();
      }
      break;

    case 10:  //changes dp raw sensor out, zero, and average functions when DP sensor is 0-10 inwc
      DP = DP10;
      DPAVG = DP10AVG;
      if (GD.inputs.tag == 6) {  //Average DP10 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 1;
        secprev = secnow;
        secelps = 0;
        myRADP10.clear();
      }
      if (GD.inputs.tag == 2) {  //zero DP10 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 1;
        dpzero();
      }
      break;

    case 24:  //changes dp raw sensor out, zero, and average functions when DP sensor is 0-24 inwc
      DP = DP24;
      DPAVG = DP24AVG;
      if (GD.inputs.tag == 6) {  //Average DP24 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 2;
        secprev = secnow;
        secelps = 0;
        myRADP24.clear();
      }
      if (GD.inputs.tag == 2) {  //zero DP24 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 2;
        dpzero();
      }
      break;

    case 138:  //changes dp raw sensor out, zero, and average functions when DP sensor is 0-138 inwc
      DP = DP138;
      DPAVG = DP138AVG;
      if (GD.inputs.tag == 6) {  //Average DP138 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 3;
        secprev = secnow;
        secelps = 0;
        myRADP138.clear();
      }
      if (GD.inputs.tag == 2) {  //zero DP138 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 3;
        dpzero();
      }
      break;
  }

  switch (spsensor) {  //changes sp raw sensor out, zero, and average functions when SP sensor is changed

    case 1:  //changes sp raw sensor out, zero, and average functions when SP sensor is 0-1 inwc
      StatP = DP01;
      SPAVG = DP01AVG;
      if (GD.inputs.tag == 7) {  //Average SP01 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 6;
        secprev = secnow;
        secelps = 0;
        myRADP01.clear();
      }
      if (GD.inputs.tag == 3) {  //zero SP01 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 4;
        dpzero();
      }
      break;

    case 5:  //changes sp raw sensor out, zero, and average functions when SP sensor is 0-5 inwc
      StatP = DP05;
      SPAVG = DP05AVG;
      if (GD.inputs.tag == 7) {  //Average SP05 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 7;
        secprev = secnow;
        secelps = 0;
        myRADP05.clear();
      }
      if (GD.inputs.tag == 3) {  //zero SP05 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 5;
        dpzero();
      }
      break;

    case 10:  //changes sp raw sensor out, zero, and average functions when SP sensor is 0-10 inwc
      StatP = DP10;
      SPAVG = DP10AVG;
      if (GD.inputs.tag == 7) {  //Average SP10 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 1;
        secprev = secnow;
        secelps = 0;
        myRADP10.clear();
      }
      if (GD.inputs.tag == 3) {  //zero SP10 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 1;
        dpzero();
      }
      break;

    case 24:  //changes sp raw sensor out, zero, and average functions when SP sensor is 0-24 inwc
      StatP = DP24;
      SPAVG = DP24AVG;
      if (GD.inputs.tag == 7) {  //Average SP24 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 2;
        secprev = secnow;
        secelps = 0;
        myRADP24.clear();
      }
      if (GD.inputs.tag == 3) {  //zero SP24 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 2;
        dpzero();
      }
      break;

    case 138:  //changes sp raw sensor out, zero, and average functions when SP sensor is 0-138 inwc
      StatP = DP138;
      SPAVG = DP138AVG;
      if (GD.inputs.tag == 7) {  //Average SP138 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 3;
        secprev = secnow;
        secelps = 0;
        myRADP138.clear();
      }
      if (GD.inputs.tag == 3) {  //zero SP138 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 3;
        dpzero();
      }
      break;
  }

  switch (nullsensor) {  //changes null raw sensor out, zero, and average functions when Null sensor is changed

    case 1:  //changes null raw sensor out, zero, and average functions when Null sensor is 0-1 inwc
      Null = DP01;
      NullAVG = DP01AVG;
      if (GD.inputs.tag == 8) {  //Average Null01 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 6;
        secprev = secnow;
        secelps = 0;
        myRADP01.clear();
      }
      if (GD.inputs.tag == 4) {  //zero Null01 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 4;
        dpzero();
      }
      break;

    case 5:  //changes null raw sensor out, zero, and average functions when Null sensor is 0-5 inwc
      Null = DP05;
      NullAVG = DP05AVG;
      if (GD.inputs.tag == 8) {  //Average Null05 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 7;
        secprev = secnow;
        secelps = 0;
        myRADP05.clear();
      }
      if (GD.inputs.tag == 4) {  //zero Null05 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 5;
        dpzero();
      }
      break;

    case 10:  //changes null raw sensor out, zero, and average functions when Null sensor is 0-10 inwc
      Null = DP10;
      NullAVG = DP10AVG;
      if (GD.inputs.tag == 8) {  //Average Null-10 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 1;
        secprev = secnow;
        secelps = 0;
        myRADP10.clear();
      }
      if (GD.inputs.tag == 4) {  //zero Null-10 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 1;
        dpzero();
      }
      break;

    case 24:  //changes null raw sensor out, zero, and average functions when Null sensor is 0-24 inwc
      Null = DP24;
      NullAVG = DP24AVG;
      if (GD.inputs.tag == 8) {  //Average Null24 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 2;
        secprev = secnow;
        secelps = 0;
        myRADP24.clear();
      }
      if (GD.inputs.tag == 4) {  //zero Null24 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 2;
        dpzero();
      }
      break;

    case 138:  //changes null raw sensor out, zero, and average functions when Null sensor is 0-138 inwc
      Null = DP138;
      NullAVG = DP138AVG;
      if (GD.inputs.tag == 8) {  //Average Null-138 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 3;
        secprev = secnow;
        secelps = 0;
        myRADP138.clear();
      }
      if (GD.inputs.tag == 4) {  //zero Null-138 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 3;
        dpzero();
      }
      break;
  }

  switch (pitchsensor) {  //changes pitch raw sensor out, zero, and average functions when Pitch sensor is changed

    case 1:  //changes pitch raw sensor out, zero, and average functions when Pitch sensor is 0-1 inwc
      Pitch1 = DP01;
      PitchAVG = DP01AVG;
      if (GD.inputs.tag == 9) {  //Average Pitch 01 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 6;
        secprev = secnow;
        secelps = 0;
        myRADP01.clear();
      }
      if (GD.inputs.tag == 5) {  //zero Pitch 01 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 4;
        dpzero();
      }
      break;

    case 5:  //changes pitch raw sensor out, zero, and average functions when Pitch sensor is 0-5 inwc
      Pitch1 = DP05;
      PitchAVG = DP05AVG;
      if (GD.inputs.tag == 9) {  //Average Pitch 01 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 7;
        secprev = secnow;
        secelps = 0;
        myRADP01.clear();
      }
      if (GD.inputs.tag == 5) {  //zero Pitch 01 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 5;
        dpzero();
      }
      break;

    case 10:  //changes pitch raw sensor out, zero, and average functions when Pitch sensor is 0-10 inwc
      Pitch1 = DP10;
      PitchAVG = DP10AVG;
      if (GD.inputs.tag == 9) {  //Average Pitch 10 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 1;
        secprev = secnow;
        secelps = 0;
        myRADP10.clear();
      }
      if (GD.inputs.tag == 5) {  //zero Pitch 10 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 1;
        dpzero();
      }
      break;

    case 24:  //changes pitch raw sensor out, zero, and average functions when Pitch sensor is 0-24 inwc
      Pitch1 = DP24;
      PitchAVG = DP24AVG;
      if (GD.inputs.tag == 9) {  //Average Pitch 24 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 2;
        secprev = secnow;
        secelps = 0;
        myRADP24.clear();
      }
      if (GD.inputs.tag == 5) {  //zero Pitch 24 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 2;
        dpzero();
      }
      break;

    case 138:  //changes pitch raw sensor out, zero, and average functions when Pitch sensor is 0-138 inwc
      Pitch1 = DP138;
      PitchAVG = DP138AVG;
      if (GD.inputs.tag == 9) {  //Average Pitch 138 Sensor
        Serial.println();
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        rec = 1;
        dprec = 3;
        secprev = secnow;
        secelps = 0;
        myRADP138.clear();
      }
      if (GD.inputs.tag == 5) {  //zero Pitch 138 Sensor
        Serial.print("Button: ");
        Serial.println(GD.inputs.tag);
        delay(145);
        sensorzero = 3;
        dpzero();
      }
      break;
  }

  GD.Tag(1);
  GD.cmd_button(10, 10, 100, 50, 30, 0, "HOME");
  GD.Tag(2);
  GD.cmd_button(5 + 115, 70 + 35, 85, 50, 29, 0, "ZERO");  //Zeros DP
  GD.Tag(3);
  GD.cmd_button(5 + 115, 70 + 105 + 35, 85, 50, 29, 0, "ZERO");  //Zero SP
  GD.Tag(4);
  GD.cmd_button(5 + 115, 70 + 105 * 2 + 35, 85, 50, 29, 0, "ZERO");  //Zeros Null
  GD.Tag(5);
  GD.cmd_button(5 + 115, 70 + 105 * 3 + 35, 85, 50, 29, 0, "ZERO");  //Zero Pitch
  GD.Tag(6);
  GD.cmd_button(5 + 24 + 220 + 100, 70 + 35, 85, 50, 29, 0, "AVG");  //AVG DP
  GD.Tag(7);
  GD.cmd_button(5 + 24 + 220 + 100, 70 + 105 + 35, 85, 50, 29, 0, "AVG");  //AVG SP
  GD.Tag(8);
  GD.cmd_button(5 + 24 + 220 + 100, 70 + 105 * 2 + 35, 85, 50, 29, 0, "AVG");  //AVG Null
  GD.Tag(9);
  GD.cmd_button(5 + 24 + 220 + 100, 70 + 105 * 3 + 35, 85, 50, 29, 0, "AVG");  //AVG Pitch

  if (GD.inputs.tag == 1) {  //home
    Serial.println();
    Serial.print("Button: ");
    Serial.println(GD.inputs.tag);
    delay(145);
    rec = 0;
    page = 1;
    Serial.print("page - ");
    Serial.println(page);
  }

  if (Null < 0.300 && Null > -0.300) {
    digitalWrite(ledPin2, HIGH);
  } else {
    digitalWrite(ledPin2, LOW);
  }

  if (Null < 0.100 && Null > -0.100) {
    digitalWrite(ledPin1, HIGH);
  } else {
    digitalWrite(ledPin1, LOW);
  }

  if (rec == 1) {
    avg();
  }
  GD.swap();
}

/*//////////////////Average Values - Record///////////////////////////////////////////////////////////////////////*/
void avg() {
  int pointIndex = dapoint - 1;
  int pipeIndex = dapipe - 1;
  int portIndex = daport - 1;
  int patestIndex = patest - 1, paportIndex = paport - 1, papointIndex = papoint - 1;
  DateTime now = rtc.now();
  secnow = now.second();
  samples++;
  Serial.print(" Samples ");
  Serial.print(samples);
  if ((secelps < secrec) && (secnow != secprev)) {
    secelps++;
    secprev++;
    if (secprev == 60) {
      secprev -= 60;
    }
    Serial.print(" Sec Elps ");
    Serial.print(secelps);
    Serial.print(" Sec Now ");
    Serial.print(secnow);
    Serial.print(" Sec Prev ");
    Serial.print(secprev);
    Serial.print(" Display Elapsed Time ");
    Serial.print(rectime);
    rectime--;
    Serial.print(" Display Elapsed Time ");
    Serial.print(rectime);
    switch (dprec) {
      case 1:  //10inwc
        myRADP10.addValue(DP10);
        Serial.print(" DP10 Value ");
        Serial.print(DP10);
        DP10AVG = myRADP10.getAverage();
        Serial.print(" DP10 AVG ");
        Serial.print(DP10AVG);
        break;
      case 2:  //24inwc
        myRADP24.addValue(DP24);
        Serial.print(" DP24 Value ");
        Serial.print(DP24);
        DP24AVG = myRADP24.getAverage();
        Serial.print(" DP24 AVG ");
        Serial.print(DP24AVG);
        break;
      case 3:  //138inwc
        myRADP138.addValue(DP138);
        Serial.print(" DP138 Value ");
        Serial.print(DP138);
        DP138AVG = myRADP138.getAverage();
        Serial.print(" DP138 AVG ");
        Serial.print(DP138AVG);
        break;
      case 4:  //Temp
        myRATP.addValue(TPE1);
        Serial.print(" Temp Value ");
        Serial.print(TPE1);
        TPAVG = myRATP.getAverage();
        Serial.print(" TEMP AVG ");
        Serial.print(TPAVG);
        break;
      case 5:  //PA DP & SP Average
        myRADP24.addValue(DP24);
        Serial.print(" DP24 Value ");
        Serial.print(DP24);
        DP24AVG = myRADP24.getAverage();
        Serial.print(" DP24 AVG ");
        Serial.print(DP24AVG);
        myRADP138.addValue(DP138);
        Serial.print(" DP138 Value ");
        Serial.print(DP138);
        DP138AVG = myRADP138.getAverage();
        Serial.print(" DP138 AVG ");
        Serial.print(DP138AVG);
        myRATP.addValue(TPE1);
        Serial.print(" Temp Value ");
        Serial.print(TPE1);
        TPAVG = myRATP.getAverage();
        Serial.print(" TEMP AVG ");
        Serial.print(TPAVG);
        break;
      case 6:  //01inwc
        myRADP01.addValue(DP01);
        Serial.print(" DP 01 Value ");
        Serial.print(DP01);
        DP01AVG = myRADP01.getAverage();
        Serial.print(" DP01 AVG ");
        Serial.print(DP01AVG);
        break;
      case 7:  //05inwc
        myRADP05.addValue(DP05);
        Serial.print(" DP 05 Value ");
        Serial.print(DP05);
        DP05AVG = myRADP05.getAverage();
        Serial.print(" DP05 AVG ");
        Serial.print(DP05AVG);
        break;
    }
  } else if (secelps >= secrec) {
    Serial.print(" Samples/Sec ");
    Serial.print(samples / secrec);
    rec = 0;
    rectime = secrec;
    samples = 0;
    switch (dprec) {
      case 1:  //10inwc
        Serial.print(" DP10 Ending AVG ");
        Serial.print(DP10AVG);
        break;
      case 2:  //24inwc
        Serial.print(" DP24 Ending AVG ");
        Serial.print(DP24AVG);
        break;
      case 3:  //138inwc
        Serial.print(" DP138 Ending AVG ");
        Serial.print(DP138AVG);
        break;
      case 4:  //Temp
        Serial.print(" Temp Ending AVG ");
        Serial.print(TPAVG);
        break;
      case 5:  //24inwc & 138inwc & temp
        Serial.print(" DP10 Ending AVG ");
        Serial.print(DP10AVG);
        Serial.print(" DP138 Ending AVG ");
        Serial.print(DP138AVG);
        break;
      case 6:  //01inwc
        Serial.print(" DP01 Ending AVG ");
        Serial.print(DP01AVG);
        break;
      case 7:  //05inwc
        Serial.print(" DP05 Ending AVG ");
        Serial.print(DP05AVG);
        break;
    }
  }

  DA[pipeIndex][portIndex][pointIndex] = DP24AVG;

  if (page == 21) {
    if (GD.inputs.tag == 4) {
      PAVH[patestIndex][paportIndex][papointIndex] = DP24AVG;
    }
    if (GD.inputs.tag == 5) {
      PASP[patestIndex][paportIndex][papointIndex] = DP138AVG;
    }
    if (GD.inputs.tag == 6) {
      PATP[patestIndex][paportIndex][papointIndex] = TPAVG;
    }
    if (GD.inputs.tag == 7) {
      PAVH[patestIndex][paportIndex][papointIndex] = DP24AVG;
      PASP[patestIndex][paportIndex][papointIndex] = DP138AVG;
      PATP[patestIndex][paportIndex][papointIndex] = TPAVG;
    }
  }
}

/*//////////////////Read Sensors - All DP///////////////////////////////////////////////////////////////////////*/
void readall() {                       //can read xx readings/second
  adc0 = ads1.readADC_SingleEnded(0);  //0-15psi 015PA Baropressure Operating max 30 psi/ BurstP 60psi
  adc4 = ads1.readADC_SingleEnded(1);  //+-01inwc 001ND 4/8/20
  adc3 = ads2.readADC_SingleEnded(1);  // 5psi 138inwc 005PD  SN5 sensors swapped
  adc1 = ads1.readADC_SingleEnded(3);  //+-10inwc 010ND 12/20
  adc2 = ads2.readADC_SingleEnded(0);  //+-60mbar 24inwc 060mb 12/14
  adc5 = ads1.readADC_SingleEnded(2);  //+-05inwc SN5 sensors swapped
  adc8 = ads2.readADC_SingleEnded(2); //Vs used for circuit voltage


  adcv0 = adc0 * multiplier;
  adcv1 = adc1 * multiplier;
  adcv2 = adc2 * multiplier;
  adcv3 = adc3 * multiplier;
  adcv4 = adc4 * multiplier;
  adcv5 = adc5 * multiplier;  //conversion to voltage
  adcv8 = adc8 * multiplier;
  cirvolt = adcv8;

  BP = ((18.75 * (adcv0) / cirvolt) - 1.875 + bpoffset) * 2.03602;  //inhg
  DP01 = 2.5 * (adcv4 / cirvolt) - 1.25 - dp01z;                    //inwc-offset
  DP05 = 12.5 * (adcv5 / cirvolt) - 6.25 - dp05z;                   //inwc-offset 5inwc
  DP10 = 25 * (adcv1 / cirvolt) - 12.5 - dp10z;                     //inwc - offset
  DP24 = ((150 * (adcv2 / cirvolt) - 75.0) * 0.401865) - dp24z;     //mbar to inwc - offset
  DP138 = ((12.5 * (adcv3 / cirvolt) - 6.25) * 27.7076) - dp138z;   //psi to inwc - offset 5psi


if (millis() > tm + 1000){
Serial.println("Cirvolt: " + String(adcv8));
  tm = millis();
}

  myBP.addValue(BP);
  myDP10.addValue(DP10);
  myDP24.addValue(DP24);
  myDP138.addValue(DP138);
  myDP01.addValue(DP01);
  myDP05.addValue(DP05);  //dampened values
}

/*//////////////////Read Sensors - BP///////////////////////////////////////////////////////////////////////*/
void readbp() {
  adc0 = ads1.readADC_SingleEnded(0);  //0-15psi 015PA Baropressure Operating max 30 psi/ BurstP 60psi
  adcv0 = adc0 * multiplier;
  BP = (((18.75 * (adcv0) / cirvolt) - 1.875) * 2.03602) + bpoffset;  //inhg
  myBP.addValue(BP);
}

/*//////////////////Read Sensors///////////////////////////////////////////////////////////////////////*/
void readtp() {

  TPE1 = TemperatureSensor[0]->readThermocouple(FAHRENHEIT);
  TPI1 = TemperatureSensor[0]->readJunction(FAHRENHEIT);
  // TPE2 = TemperatureSensor[1]->readThermocouple(FAHRENHEIT);
  // TPI2 = TemperatureSensor[1]->readJunction(FAHRENHEIT);

  switch ((int)TPE1) {
    case FAULT_OPEN:
      //    Serial.print("FAULT_OPEN");//Temp will display 0.00 if FAULT_OPEN
      TPE1 = 0.0;
      break;
    case FAULT_VOLTAGE:
      // Serial.print("FAULT_VOLTAGE"); // Temp will display 1.00 if FAULT_VOLTAGE
      TPE1 = 1.0;
      break;
    case NO_MAX31856:
      // Serial.print("NO_MAX31856"); // Temp will display 2.00 if NO_MAX31856
      TPE1 = 2.0;
      break;
    default:
      //     Serial.print(TPE);
      break;
  }
  switch ((int)TPE2) {
    case FAULT_OPEN:
      //    Serial.print("FAULT_OPEN");//Temp will display 0.00 if FAULT_OPEN
      TPE2 = 0.0;
      break;
    case FAULT_VOLTAGE:
      // Serial.print("FAULT_VOLTAGE"); // Temp will display 1.00 if FAULT_VOLTAGE
      TPE2 = 1.0;
      break;
    case NO_MAX31856:
      // Serial.print("NO_MAX31856"); // Temp will display 2.00 if NO_MAX31856
      TPE2 = 2.0;
      break;
    default:
      //     Serial.print(TPE);
      break;
  }
}
/*//////////////////Zero - DP10/24/138///////////////////////////////////////////////////////////////////////*/
void dpzero() {
  Serial.print("DP Zero ");
  Serial.print("Case: ");
  Serial.println(sensorzero);
  switch (sensorzero) {
    case 1:  //DP10
      myRADP10Z.clear();
      for (int i = 1; i < 21; i++) {
        dp10z = ads1.readADC_SingleEnded(3) * 0.1875F / 1000;  //+-10inwc 010ND 12/20
        myRADP10Z.addValue(dp10z);
        Serial.print("Count\t");
        Serial.print(i);
        Serial.print("\tDP\t");
        Serial.println(dp10z, 4);
      }
      dp10z = 25 * (myRADP10Z.getAverage() / cirvolt) - 12.5;  //inwc
      Serial.print("\tDP Offset: ");
      Serial.print(dp10z, 4);
      Serial.println(" inwc");
      EEPROM.updateFloat(dp10zeroadr, dp10z);
      dp10z = EEPROM.readFloat(dp10zeroadr);
      Serial.print("DP 10 Zero Value: ");
      Serial.println(EEPROM.readFloat(dp10zeroadr), 4);
      sensorzero = 0;
      break;
    case 2:  //DP24
      myRADP24Z.clear();
      for (int i = 1; i < 21; i++) {
        dp24z = ads2.readADC_SingleEnded(0) * 0.1875F / 1000;  //+-10inwc 010ND 12/20
        myRADP24Z.addValue(dp24z);
        Serial.print("Count\t");
        Serial.print(i);
        Serial.print("\tDP\t");
        Serial.println(dp24z, 4);
      }
      dp24z = (150 * (adcv2 / cirvolt) - 75.0) * 0.401865;  //inwc
      Serial.print("\tDP Offset: ");
      Serial.print(dp24z, 4);
      Serial.println(" inwc");
      EEPROM.updateFloat(dp24zeroadr, dp24z);
      dp24z = EEPROM.readFloat(dp24zeroadr);
      Serial.print("DP 24 Zero Value: ");
      Serial.println(EEPROM.readFloat(dp24zeroadr), 4);
      sensorzero = 0;
      break;
    case 3:  //DP138
      myRADP138Z.clear();
      for (int i = 1; i < 21; i++) {
        dp138z = ads2.readADC_SingleEnded(1) * 0.1875F / 1000;  //+-10inwc 010ND 12/20
        myRADP138Z.addValue(dp138z);
        Serial.print("Count\t");
        Serial.print(i);
        Serial.print("\tDP\t");
        Serial.println(dp138z, 4);
      }
      dp138z = (12.5 * (adcv3 / cirvolt) - 6.25) * 27.7076;  //inwc
      Serial.print("\tDP Offset: ");
      Serial.print(dp138z, 4);
      Serial.println(" inwc");
      EEPROM.updateFloat(dp138zeroadr, dp138z);
      dp138z = EEPROM.readFloat(dp138zeroadr);
      Serial.print("DP 138 Zero Value: ");
      Serial.println(EEPROM.readFloat(dp138zeroadr), 4);
      sensorzero = 0;
      break;
    case 4:  //DP01
      myRADP01Z.clear();
      for (int i = 1; i < 21; i++) {
        dp01z = ads1.readADC_SingleEnded(1) * 0.1875F / 1000;
        myRADP01Z.addValue(dp01z);
        Serial.print("Count\t");
        Serial.print(i);
        Serial.print("\tDP\t");
        Serial.println(dp01z, 4);
      }
      dp01z = (2.5 * (adcv4 / cirvolt) - 1.25);  //inwc
      Serial.print("\tDP Offset: ");
      Serial.print(dp01z, 4);
      Serial.println(" inwc");
      EEPROM.updateFloat(dp01zeroadr, dp01z);
      dp01z = EEPROM.readFloat(dp01zeroadr);
      Serial.print("DP 01 Zero Value: ");
      Serial.println(EEPROM.readFloat(dp01zeroadr), 4);
      sensorzero = 0;
      break;
    case 5:  //DP05
      myRADP05Z.clear();
      for (int i = 1; i < 21; i++) {
        dp05z = ads1.readADC_SingleEnded(2) * 0.1875F / 1000;
        myRADP05Z.addValue(dp05z);
        Serial.print("Count\t");
        Serial.print(i);
        Serial.print("\tDP\t");
        Serial.println(dp05z, 4);
      }
      dp05z = (12.5 * (adcv5 / cirvolt) - 6.25);  //inwc
      Serial.print("\tDP Offset: ");
      Serial.print(dp05z, 4);
      Serial.println(" inwc");
      EEPROM.updateFloat(dp05zeroadr, dp05z);
      dp05z = EEPROM.readFloat(dp05zeroadr);
      Serial.print("DP 05 Zero Value: ");
      Serial.println(EEPROM.readFloat(dp05zeroadr), 4);
      sensorzero = 0;
      break;
  }
}
