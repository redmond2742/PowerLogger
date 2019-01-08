

/*
-----This code is in the public domain free to use and remix----

Resources
http://www.hacktronics.com/Tutorials/arduino-current-sensor.html
For Time.h library
Download Time.zip (https://www.pjrc.com/teensy/td_libs_Time.html)
    Import: Arduino->Sketch->add .ZIP Library-> Select Time.zip from download
Uses the ACS715 Hall Effect Current Sensor
*/


#include <SPI.h>
#include <Wire.h>
#include <SD.h>
#include <Time.h>

File myFile;

long readVcc() {
  long result; 
  ADMUX = _BV(REFS0)  | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);
  ADCSRA |= _BV(ADSC); //convert
  while (bit_is_set(ADCSRA,ADSC));
  result = ADCL;
  result |= ADCH <<8;
  result = 1125300L / result;
  return result;
}

void digitalClockDisplay(){
  // digital clock display of the time
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year()); 
  Serial.print(" ");
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
   
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if(digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

int batMonPin = A0;    // input pin for the voltage divider
int batVal = 0;       // variable for the A/D value
float pinVoltage = 0; // variable to hold the calculated voltage
float batteryVoltage = 0;

unsigned int ADCValue;
double Voltage;
double Vcc;

int analogInPin = A1;  // Analog input pin that the carrier board OUT is connected to
int sensorValue = 0;        // value read from the carrier board
int outputValue = 0;        // output in milliamps
unsigned long msec = 0;
float time = 0.0;
int sample = 0;
float totalCharge = 0.0;
float averageAmps = 0.0;
float ampSeconds = 0.0;
float ampHours = 0.0;
float wattHours = 0.0;
float amps = 0.0;

int R1 = 9890; // Resistance of R1 in ohms
int R2 = 4620; // Resistance of R2 in ohms
float ratio = 0;  // Calculated from R1 / R2

void setup() {
  Serial.begin(9600);
  setTime(0,0,0,11,5,2015);
  Serial.print("Initializing SD card....");
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  
   if(!SD.begin(4)){
    Serial.println("initialization failed!");
    return;
   }
    Serial.println("initialization done.");
    myFile = SD.open("PowerLog.txt", FILE_WRITE);
    
    if(myFile){
  Serial.print("Writing to testLog.txt...");
  myFile.println("testing data log");
  //Close the file writer
    myFile.close();
}else{
  Serial.println("error opening text file");
}
}

void loop() {
digitalClockDisplay();
int sampleBVal = 0;
int avgBVal = 0; 
int sampleAmpVal = 0;
int avgSAV = 0;

for (int x = 0; x < 10; x++){ // run through loop 10x

  // read the analog in value:
  
 // batVal = analogRead(batMonPin);    // read the voltage on the divider
 // sampleBVal = sampleBVal + batVal; // add samples together
  sensorValue = analogRead(analogInPin);  
  sampleAmpVal = sampleAmpVal + sensorValue; // add samples together
 
  delay (10); // let ADC settle before next sample
}

//avgBVal = sampleBVal / 10; //divide by 10 (number of samples) to get a steady reading
avgSAV = sampleAmpVal / 10;



  
/* sensor outputs about 100 at rest.
Analog read produces a value of 0-1023, equating to 0v to 5v.
"((long)sensorValue * 5000 / 1024)" is the voltage on the sensor's output in millivolts.
There's a 500mv offset to subtract.
The unit produces 133mv per amp of current, so
divide by 0.133 to convert mv to ma
         
*/
batVal = analogRead(batMonPin);
Vcc = readVcc()/1000.0;
pinVoltage = (batVal/1023.0)*Vcc;
ratio = ( ((float)R1+(float)R2)/(float)R2)/.96962; //.96962 is calculate fudge factor
batteryVoltage = pinVoltage * ratio; 


//sensorValue = analogRead(analogInPin);
//sampleAmpVal = sampleAmpVal + sensorValue;
outputValue = (((long)avgSAV * 5000 / 1024) - 494 ) * 1000 / 133; 
 
amps = (float) outputValue / 1000;
float watts = amps * batteryVoltage;

  
  //Serial.print(dateString);
  Serial.print("\t ");
  Serial.print("Volts = " );
  Serial.print(batteryVoltage);
  Serial.print("\t Current (amps) = ");
  Serial.print(amps);
  Serial.print("\t Power (Watts) = ");  
  Serial.print(watts); 
  
  sample = sample + 1;
 
  msec = millis();
 
  time = (float) msec / 1000.0;
 
  totalCharge = totalCharge + amps;
 
  averageAmps = totalCharge / sample;
  ampSeconds = averageAmps*time;
  
  ampHours = ampSeconds/3600;
 
  wattHours = batteryVoltage * ampHours;
  
  Serial.print("\t Time (hours) = ");
  Serial.print(time/3600);
 
  Serial.print("\t Amp Hours (ah) = ");
  Serial.print(ampHours);
  Serial.print("\t Watt Hours (wh) = ");
  Serial.println(wattHours);
  
   //Open the file writer
  myFile = SD.open("PowerLog.txt", FILE_WRITE);  
 if(myFile){
    //myFile.print(dateString);
    myFile.print("Volts = " );                      
    myFile.print(batteryVoltage);
    myFile.print("\t Current (amps) = ");
    myFile.print(amps);
    myFile.print("\t Power (Watts) = "); 
    myFile.print(watts);
    myFile.print("\t Time (hours) = ");
    myFile.print(time/3600);
    myFile.print("\t Amp Hours (ah) = ");
    myFile.print(ampHours);
    myFile.print("\t Watt Hours (wh) = ");
    myFile.print(wattHours);
    myFile.print("\r\n");
    
    //Close the file writer
    myFile.close();
  
 }
  
  delay(1000);
}
