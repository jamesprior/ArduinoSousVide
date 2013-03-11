// The shield uses the I2C SCL and SDA pins. On classic Arduinos
// this is Analog 4 and 5 so you can't use those for analogRead() anymore
// However, you can connect other I2C sensors to the I2C bus and share
// the I2C bus.
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();


// These #defines make it easy to set the backlight color
#define WHITE 0x7

#define TEMP_WIDTH 6 // the number of characters reserved for displaying a temperature, includes unit
#define STATUS_LINE_COUNT 5 // total number status lines

byte currentSetCol=2;
char tempUnit = 'C';
int lastPidStepNumber;
byte topStatusLine = 0;
uint8_t buttons;

void startStatusMode(){
  lcd.noBlink();
  lcd.clear();
  inSetTempMode = false;
  writeStatus();
}

void writeStatus(){
  if (topStatusLine == 255) { topStatusLine = STATUS_LINE_COUNT-1;}
  if (topStatusLine == STATUS_LINE_COUNT) { topStatusLine = 0;}
  
  lcd.setCursor(0,0);
  lcd.print(statusLine(topStatusLine));
  lcd.setCursor(0,1);
  lcd.print(statusLine(((topStatusLine + 1) % STATUS_LINE_COUNT)));  
}

String statusLine(byte forIndex){
  float tempVariance ;
  // noew, set, variance, relay status, pid status
  switch(forIndex){
    case 0:
      return String("Now: ")+tempAsString(currentTempC);
      break;
    case 1:
      return  String("Set: ")+tempAsString(pidSetpoint);
      break;
    case 2:
      tempVariance = pidSetpoint-currentTempC;
      return String("Variance: ") + formatTemp(abs(tempVariance));
      break;
    case 3:
      if (outletState == HIGH){
        return String("Relay: on ");
      } else {
        return String("Relay: off");
      }
      break;
    case 4:
    // TODO, this.  Get the step number and the output in a string
      return String("s: ")+String(pidStepNumber)+String(" d: ")+formatTemp(pidOutput);
      break;
    default:
      return String("N/A");
      break;
    }
    
}


void LCDControlSetup() {
  // Debugging output
  // set up the LCD's number of columns and rows: 
  lcd.begin(16, 2);
  lcd.setBacklight(WHITE);
  startStatusMode();
}

String formatTemp(float tempAsC){
  char floatString[10];
  dtostrf(tempAsC,TEMP_WIDTH-1,1,floatString);
  return String(floatString)+tempUnit;
}

// Returns a five character string
String tempAsString(float temp){
  if (tempUnit == 'F'){
    temp = DallasTemperature::toFahrenheit(temp);
  }
  return formatTemp(temp);

}
      

// for changing a set temperature, given the current column 
// and the column the curosor was at when the temp was printed, calculate
// the scale to change it by
float getTempInputScaleChange(byte currentCol, byte startCol){
  //this expects the temperature to look something like " 99.8"
  if (currentCol - startCol == 0){
    return 100;
  } else if (currentCol - startCol == 1){
    return 10;
  } else if (currentCol - startCol == 2){
    return 1;
  } else if (currentCol - startCol == 4){
    return 0.1;
  } else {
    return 0;
  } 
}

void toggleTempUnit(){
  if (tempUnit == 'C'){
    tempUnit = 'F';
  } else {
    tempUnit = 'C';
  }
}

void handleTempSetInput(uint8_t buttons){
  if (buttons & BUTTON_UP){
    if (currentSetCol == TEMP_WIDTH-1){ // col is zero indexed, so this is the F/C designation
      toggleTempUnit(); 
    }
    pidSetpoint = pidSetpoint + getTempInputScaleChange(currentSetCol, 0); // The temp is the first thing on the row, zero offset
    lcd.setCursor(0,1);
    lcd.print(tempAsString(pidSetpoint));
  }
  if (buttons & BUTTON_DOWN){
    if (currentSetCol == TEMP_WIDTH-1){
      toggleTempUnit();
    }
    pidSetpoint = pidSetpoint - getTempInputScaleChange(currentSetCol, 0);
    lcd.setCursor(0,1);
    lcd.print(tempAsString(pidSetpoint));
  }
  if (buttons & BUTTON_RIGHT){
    currentSetCol++;
    if (currentSetCol == 3) { // magic number for decimal
      currentSetCol++;
    }
  }
  if (buttons & BUTTON_LEFT){
    currentSetCol --;
    if (currentSetCol == 3) { // magic number for decimal
      currentSetCol--;
    }
  }
  if (buttons & BUTTON_SELECT) {
    endTempSetInput();
  }

//  if (currentSetCol == 255 || (currentSetCol == 0 && pidSetPoint < 100.0)) { // we hit left to often and it wrapped
  if (currentSetCol == 255 || (currentSetCol == 0 && pidSetpoint < 100.0)) { // we hit left to often and it wrapped
    currentSetCol = (TEMP_WIDTH-1);
  } else {
    currentSetCol = currentSetCol % (TEMP_WIDTH); // last col is changing the temp unit
  }
  
  lcd.setCursor(currentSetCol,1);
  
}

void startTempSetInput(){
  inSetTempMode = true;
  lcd.clear();
  lcd.blink();
  lcd.setCursor(0,0);
  lcd.print("Set Temp:");
  lcd.setCursor(0,1);
  lcd.print(tempAsString(pidSetpoint));
  
  lcd.setCursor(currentSetCol,1);
}

void endTempSetInput(){
  inSetTempMode = false;
  startStatusMode();
  resetPidControlLoop();
}
  

void LCDControlLoop() {
    // Only do something when the buttons are released
  uint8_t buttonsNow = lcd.readButtons();
  
  if (buttonsNow) {
    buttons = buttonsNow;    
  }
  else if(buttons) {
    if (inSetTempMode){
      handleTempSetInput(buttons);
    }
    else {
      if (buttons & BUTTON_SELECT) {
        startTempSetInput();
      }
      if (buttons & BUTTON_UP) {
        topStatusLine++;
        lcd.clear();
        writeStatus();
      }
      if (buttons & BUTTON_DOWN) {
        topStatusLine--;
        lcd.clear();
        writeStatus();
      }
    }
    //very important:  Cleanup
    buttons = 0;
  }
  else {
    if (!inSetTempMode){
      if (pidStepNumber != lastPidStepNumber){
        lastPidStepNumber = pidStepNumber;
        writeStatus();
      }
    }
  }
}



