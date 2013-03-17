#include <PID_v1.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_RGBLCDShield.h>


//Arduino uses analog pins 4 and 5 to communicate with the LCD screen
//These pins cannot be used for general i/o.

// DEFINED PINS
#define ledStatusPin 13       // the number of the output pin
#define relayControlPin 2    // the relay control pin, high turns the outlet on
#define extraPowerPin 13

OneWire  ds(10);  // Temperature control on pin 10
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&ds);
DeviceAddress jbWeldTherm;

// PID control
// previously window 5000, steps 5, kp=3, ki=0.5, kd=2
double pidSetpoint, currentTempC, pidOutput;  // variables we'll be using for PID control
double kp=3,ki=0.4,kd=1.5;
PID myPID(&currentTempC, &pidOutput, &pidSetpoint,kp,ki,kd, DIRECT);  //Specify the links and initial tuning parameters
int pidWindowSize = 10000; // the control window for the relay, 10 seconds
int pidWindowSteps = 5; // 2 second increments
int pidWindowStepSize = pidWindowSize / pidWindowSteps;
int pidStepNumber = 0;
unsigned long lastStepTime;
unsigned long now;
int outletState = HIGH;      // the current state of the outlet, HIGH=on
boolean inSetTempMode = false;

void resetPidControlLoop(){
  // the PID relay control window
  pidStepNumber = 1;
  sensors.requestTemperatures(); // Send the command to get temperatures
  currentTempC = sensors.getTempC(jbWeldTherm);
  myPID.Compute();
  lastStepTime = millis();
}

void setup()
{
  pinMode(ledStatusPin, OUTPUT);
  pinMode(relayControlPin, OUTPUT);
  
  pinMode(extraPowerPin, OUTPUT);
  digitalWrite(extraPowerPin, HIGH);

  Serial.begin(9600);
  
  LCDControlSetup();
  
  //start the dallas reader
  sensors.begin();
  if (!sensors.getAddress(jbWeldTherm, 0)) Serial.println("Unable to find address for Device 0"); 
  sensors.setResolution(jbWeldTherm, 11);
 
  //initialize the variables we're linked to
  pidSetpoint = 63.0; // we're working in celsius

  //tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, pidWindowSteps+1);

  //turn the PID on
  myPID.SetMode(AUTOMATIC);
  
  resetPidControlLoop();
  
  // prints title with ending line break 
  Serial.println("BOOT!"); 
}

void loop()
{
  if (inSetTempMode) {
    outletState = LOW;
  }
  else{
    now = millis();

    /************************************************
     * turn the output pin on/off based on pid output
     ************************************************/  
    if (now > lastStepTime + pidWindowStepSize){
      sensors.requestTemperatures(); // Send the command to get temperatures
      currentTempC = sensors.getTempC(jbWeldTherm);
  
      //Serial.println("Increasing step number due to time");
      lastStepTime = now;
      pidStepNumber += 1;
    }
   
    if(pidStepNumber > pidWindowSteps)
    { //time to reset our steps
      pidStepNumber = 1;
      myPID.Compute();  
    }
  
  
    if(pidOutput > pidStepNumber) outletState = HIGH;
    else outletState = LOW;
  }
  
  digitalWrite(relayControlPin,outletState);
  digitalWrite(ledStatusPin,outletState);
  
  LCDControlLoop();
  

}



