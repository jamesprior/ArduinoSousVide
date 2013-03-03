#include <PID_v1.h>
#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <stdlib.h>

// no-cost stream operator as described at 
// http://arduiniana.org/libraries/streaming/
template<class T>
inline Print &operator <<(Print &obj, T arg)
{ 
  obj.print(arg); 
  return obj; 
}

//Arduino uses digital pins 10, 11, 12, and 13 (SPI) to communicate with the W5100 on the ethernet shield. 
//These pins cannot be used for general i/o.

//TODO debug the steps and the timing, it seems to be clicking more than I thought given the window size and steps

// DEFINED PINS
#define ledStatusPin 8       // the number of the output pin
#define relayControlPin 2    // the relay control pin, high turns the outlet on

OneWire  ds(4);  // Temperature control on pin 4
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
unsigned long pidWindowStartTime;
char currentTempString[8];
int outletState = HIGH;      // the current state of the outlet, HIGH=on
unsigned long lastRelayToggle = millis(); // we only want to toggle the relay once per second

/* This creates an instance of the webserver.  By specifying a prefix
 * of "/", all pages will be at the root of the server. */
#define PREFIX "/"
WebServer webserver(PREFIX, 80);

void setup()
{
  pinMode(ledStatusPin, OUTPUT);
  pinMode(relayControlPin, OUTPUT);

  Serial.begin(9600);
  
  webServerSetup();
  
  //start the dallas reader
  sensors.begin();
  if (!sensors.getAddress(jbWeldTherm, 0)) Serial.println("Unable to find address for Device 0"); 
  sensors.setResolution(jbWeldTherm, 11);
 
  
  // the PID relay control window
  pidWindowStartTime = millis();

  //initialize the variables we're linked to
  pidSetpoint = 37.77; // we're working in celsius

  //tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, pidWindowSteps+1);

  //turn the PID on
  myPID.SetMode(AUTOMATIC);
  
  // prints title with ending line break 
  Serial.println("BOOT!"); 
}

void loop()
{
  sensors.requestTemperatures(); // Send the command to get temperatures
  
  currentTempC = sensors.getTempC(jbWeldTherm);

  /* process incoming connections one at a time forever */
  webserver.processConnection();
  
//  Serial.print("Set temp is: ");
//  Serial.print(pidSetpoint);
//  Serial.print(", current is: ");
//  Serial.println(currentTempC);
//  
  unsigned long now = millis();
  Serial.print("Start loop at ");
  Serial.print(now);
  Serial.print(", lastStepTime:");
  Serial.print(lastStepTime);
  Serial.print(", pidStepNumber:");
  Serial.print(pidStepNumber);
  Serial.print(", pidWindowStepSize:");
  Serial.print(pidWindowStepSize);
  Serial.print(", pidOutput:");
  Serial.print(pidOutput);
  Serial.print("\n");
  
  /************************************************
   * turn the output pin on/off based on pid output
   ************************************************/  
  if (now > lastStepTime + pidWindowStepSize){
    //Serial.println("Increasing step number due to time");
    lastStepTime = now;
    pidStepNumber += 1;
  }
 
  if(pidStepNumber > pidWindowSteps)
  { //time to reset our steps
    pidWindowStartTime = now;
    pidStepNumber = 1;
    myPID.Compute();  
  }


  if(pidOutput > pidStepNumber) outletState = HIGH;
  else outletState = LOW;
  
  digitalWrite(relayControlPin,outletState);
  digitalWrite(ledStatusPin,outletState);
  
  Serial.print("Finishing as lastStepTime:");
  Serial.print(lastStepTime);
  Serial.print(", pidStepNumber:");
  Serial.print(pidStepNumber);
  Serial.print(", pidWindowStepSize:");
  Serial.print(pidWindowStepSize);
  Serial.print(", pidOutput:");
  Serial.print(pidOutput);
  Serial.print("\n");

}



