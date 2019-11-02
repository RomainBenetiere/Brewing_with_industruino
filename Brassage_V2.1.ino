
/*
* Industruino Demo Code - Default code loaded onto Industruino
*
* Copyright (c) 2013 Loic De Buck <connect@industruino.com>
*
* Industruino is a DIN-rail mountable Arduino Leonardo compatible product
* Please visit www.industruino.com for further information and code examples.
* Standard peripherals connected to Industruino are:
* UC1701 compatible LCD; rst:D19 dc:D20 dn:D21 sclk:D22 (set pin configuration in UC1701 library header)
* 3-button membrane panel; D23, D24, D25
*/
#include <Indio.h>  
#include <Wire.h>


#include <UC1701.h>
//Download libary from https://github.com/Industruino/

// A custom glyph (a smiley)...
static const byte glyph[] = { B00010000, B00110100, B00110000, B00110100, B00010000 };


static UC1701 lcd;

//menu defines

//- initial cursor parameters
int coll = 0; //column counter for cursor - always kept at 0 in this demo (left side of the screen)
int channel = 0; //Counter is controlled by the up&down buttons on the membrane panel. Has double use; 1. As row controller for the cursor (screen displays 6 rows of text, counting from 0 to 5). 2. As editor for numerical values shown on screen
int lastChannel = 0; //keeps track of previous 'channel'. Is used to detect change in state.

//- initial menu level parameters
int MenuLevel = 0; //Defines the depth of the menu tree
int MenuID = 0; //Defines the unique identifier of each menu that resides on the same menu level
int channelUpLimit = 5; //Defines the upper limit of the button counter: 1. To limit cursor's downward row movement 2. To set the upper limit of value that is beeing edited.
int channelLowLimit = 0; //Defines the lower limit of the button counter: 1. To limit cursor's upward row movement 2. To set the lower limit of value that is beeing edited.

//- initial parameters for 'value editing mode'
int valueEditing = 0; //Flag to indicate if the interface is in 'value editing mode', thus disabling cursor movement.
int row = 0; //Temporary location to store the current cursor position whilst in 'value editing mode'.
int constrainEnc = 1; //Enable/disable constraining the button panel's counter to a lower and upper limit.
float valueEditingInc = 0; //Increments of each button press when using 'value editing mode'.
float TargetValue = 0; // Target value to be edited in 'value editing mode'

//Membrane panel button defines

int buttonUpState = 0; //status of "Up" button input
int buttonEnterState = 0; //status of "Enter" button input
int buttonDownState = 0; //status of "Down" button input

int prevBtnUp = 0; //previous state of "Up" button
int prevBtnEnt = 0; //previous state of "Enter" button
int prevBtnDown = 0; //previous state of "Down" button

int lastBtnUp = 0; //time since last "Up" pressed event
int lastBtnEnt = 0; //time since last "Enter" pressed event
int lastBtnDown = 0; //time since last "Down" pressed event

int enterPressed = 0; //status of "Enter" button after debounce filtering : 1 = pressed 0 = unpressed

int transEntInt = 250; //debounce treshold for "Enter" button
int transInt = 100; //debounce for other buttons
unsigned long lastAdminActionTime = 0; //keeps track of last button activity

// These constants won't change.  They're used to give names
// to the pins used:
const int analogInPin = A5;  // Analog input pin that the button panel is attached to
const int backlightPin = 26; // PWM output pin that the LED backlight is attached to
const int buttonEnterPin = 24;
const int buttonUpPin = 25;
const int buttonDownPin = 23;
const int D0 = 0;
const int D1 = 1;
const int D2 = 2;
const int D3 = 3;
const int D4 = 4;
const int D5 = 5;
const int D6 = 6;
const int D7 = 7;
const int D8 = 8;
const int D9 = 9;
const int D10 = 10;
const int D11 = 11;
const int D12 = 12;
const int D14 = 14;
const int D15 = 15;
const int D16 = 16;
const int D17 = 17;

float anOutCh1 = 0;
float anOutCh2 = 0;
int anOutUpLimit = 0;

int ButtonsAnalogValue = 0;        // value read from mebrane panel buttons.
int backlightIntensity = 5;        // LCD backlight intesity
int backlightIntensityDef = 5;     // Default LCD backlight intesity
unsigned long lastLCDredraw = 0;   // keeps track of last time the screen was redrawn

// Temperature de brassage
const int TempEmp = 52;
const int TempEz1 = 62;
const int TempEz2 = 68;
const int TempInZ = 80;

float Temp1;
float Temp2;
const float Hyst = 1;

void setup() {

  Indio.analogWriteMode(1, mA);
  Indio.analogWriteMode(2, mA);
  Indio.analogWrite(1, 0, true);
  Indio.analogWrite(2, 0, true);

  SetInput(); //Sets all general pins to input
  pinMode(buttonEnterPin, INPUT);
  pinMode(buttonUpPin, INPUT);
  pinMode(buttonDownPin, INPUT);
  pinMode(backlightPin, OUTPUT); //set backlight pin to output
  analogWrite(backlightPin, (map(backlightIntensity, 5, 1, 255, 0))); //convert backlight intesity from a value of 0-5 to a value of 0-255 for PWM.
  //LCD init
  lcd.begin();  //sets the resolution of the LCD screen

  for (int y = 0; y <= 7; y++) {
    for (int x = 0; x <= 128; x++) {
      lcd.setCursor(x, y);
      lcd.print(" ");
    }
  }

  //debug
  Serial.begin(9600); //enables Serial port for debugging messages

  //Menu init
  MenuWelcome(); //load first menu

}

void loop() {

  ReadButtons(); //check buttons
  Navigate(); //update menus and perform actions
  //  delay(50);
}


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
//UI menu content - edit, add or remove these functions to make your own menu structure
//These functions only generate the content that is printed to the screen, please also edit the "Navigate" function further below to add actions to each menu.
//------------------------------------------------------------------------------------------------------------------------------------------------------------


void MenuWelcome() { //this function draws the first menu - splash screen
  //menu inintialisers
  channel = 0; //starting row position of the cursor (top row) - controlled by the button panel counter
  channelUpLimit = 0; //upper row limit
  channelLowLimit = 0; //lower row limit
  MenuLevel = 0; //menu tree depth -> first level
  MenuID = 0; //unique menu id -> has to be unique for each menu on the same menu level.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over from the previous menu
  lcd.clear(); //clear the screen
  //actual user content on the screen
  lcd.setCursor(5, 1); //set the cursor to the fifth pixel from the left edge, third row.
  lcd.print("Bienvenue a nos"); //print text on screen
  lcd.setCursor(5, 2); //set the cursor to the fifth pixel from the left edge, third row.
  lcd.print("Supers Brasseurs!"); //print text on screen
  lcd.setCursor(5, 3); //set the cursor to the fifth pixel from the left edge, third row.
  lcd.print("Fabien et Romain"); //print text on screen
  lcd.setCursor(5, 4); //set the cursor to the fifth pixel from the left edge, third row.
  lcd.print("The LAst Hop Project"); //print text on screen
  delay(4000);
}


void MenuSelect() { //second menu - choice of submenu's
  channel = 3; //starting row position of the cursor (top row) - controlled by the button panel counter
  channelLowLimit = 3;
  channelUpLimit = 4; //upper row limit
  MenuLevel = 1; //menu tree depth -> second level
  MenuID = 1; //unique menu id -> has to be unique for each menu on the same menu level.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over from the previous menu
  lcd.clear(); //clear the screen
  ScrollCursor(); //enable the moving cursor (note that this function is not called in the splash screen, thus disabling the cursor)
  //actual user content on the screen
  lcd.setCursor(6, 0); //set the cursor to the sixth pixel from the left edge, first row.
  lcd.print("Merci de select."); //print text on screen
  lcd.setCursor(6, 1); //set the cursor to the sixth pixel from the left edge, first row.
  lcd.print("une option:"); //print text on screen
  lcd.setCursor(6, 3); //set the cursor to the sixth pixel from the left edge, second row.
  lcd.print("Brassage"); //print text on screen
  lcd.setCursor(6, 4); //set the cursor to the sixth pixel from the left edge, third row.
  lcd.print("Mise a echelle"); //print text on screen
}

void MenuSetup() { //submenu of Main menu - setup screen for Industruino
  channel = 0;
  channelUpLimit = 2;
  channelLowLimit = 0;
  MenuID = 9;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("BackLight     ");
  lcd.setCursor(65, 0);
  lcd.print(backlightIntensity, 1);
  lcd.setCursor(6, 1);
  lcd.print("Reset param.");
  lcd.setCursor(6, 2);
  lcd.print("Back");
}

void MenuParametersReset() {
  channel = 6;
  channelUpLimit = 5;
  channelLowLimit = 4;
  MenuID = 10;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Set system");
  lcd.setCursor(6, 1);
  lcd.print("to default");
  lcd.setCursor(6, 2);
  lcd.print("settings?");
  lcd.setCursor(6, 4);
  lcd.print("OK?");
  lcd.setCursor(6, 5);
  lcd.print("Cancel");

}

void MenuMiseEchelle() {
  channel = 0;
  channelUpLimit = 1;
  channelLowLimit = 0;
  MenuID = 3;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Option non disponible");
  lcd.setCursor(6, 1);
  lcd.print("Back");
}

//---------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------
  
  void MenuBrassage() {
  channel = 0;
  channelUpLimit = 7;
  channelLowLimit = 0;
  MenuID = 11;
  MenuLevel = 2;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Empattage"); //"Empattage"
  lcd.setCursor(6, 1);
  lcd.print("Enzymatique 1"); //"Enzymatique 1"
  lcd.setCursor(6, 2);
  lcd.print("Enzymatique 2"); //"Enzymatique 2"
  lcd.setCursor(6, 3);
  lcd.print("Inactivation Ez"); //"Inactivation Ez"
  lcd.setCursor(6, 4);
  lcd.print("Forcage Pompe"); //"Forcage Pompe"
  lcd.setCursor(6, 5);
  lcd.print("Non utilisé"); //"Non utilisé"
  lcd.setCursor(6, 6);
  lcd.print("LCD backlight");
  lcd.setCursor(6, 7);
  lcd.print("Back");
}

  void ProEmpattage() {
  Indio.setADCResolution(14);
  Indio.analogReadMode(1, mA_p);
  Indio.analogReadMode(2, mA_p);
  Indio.digitalMode(1,OUTPUT);
  Indio.digitalMode(2,OUTPUT);
  channel = 7;
  channelUpLimit = 7;
  channelLowLimit = 7;
  MenuID = 14;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  ProEmpattageLive();
}

  void ProEmpattageLive(){ // Affichage valeur et graphique
  lcd.setCursor(6, 0);
  lcd.print("Empattage");
  Temp1=(Indio.analogRead(1))*150/100*1.0052+0.7893; // mise à l'échelle et correction
  Temp2=(Indio.analogRead(2))*200/100*1.0065+1.164; // mise à l'échelle et correction
  if ((Temp1 >= 0 and Temp1 < (TempEmp - Hyst)) or (Temp2 >=0 and Temp2 < (TempEmp - Hyst))) {
    Indio.digitalWrite(1, HIGH); // Pompe 1
    Indio.digitalWrite(2, LOW); // Pompe 2
  }
  else if (Temp1 > (TempEmp + Hyst) and Temp2 > TempEmp) {
    Indio.digitalWrite(1, LOW); // Pompe 1
    Indio.digitalWrite(2, LOW); // Pompe 2
  }
  else {
    Indio.digitalWrite(1, LOW); // Pompe 1
    Indio.digitalWrite(2, LOW); // Pompe 2 
  }

  lcd.setCursor(6, 1);
  lcd.print("Set Point");
  lcd.setCursor(67, 1);
  lcd.print(TempEmp);
  lcd.setCursor(120, 1);
  lcd.print(" C ");
  lcd.setCursor(6, 2);
  lcd.print("Temp 1");
  lcd.setCursor(67, 2);
  lcd.print(Temp1);
  lcd.setCursor(120, 2);
  lcd.print(" C ");
  lcd.setCursor(6, 3);
  lcd.print("Temp 2");
  lcd.setCursor(67, 3);
  lcd.print(Temp2);
  lcd.setCursor(120, 3);
  lcd.print(" C ");
  lcd.setCursor(6, 7);
  lcd.print("Back   ");
  
}

  void ProEnzy1() {
  Indio.setADCResolution(14);
  Indio.analogReadMode(1, mA_p);
  Indio.analogReadMode(2, mA_p);
  channel = 7;
  channelUpLimit = 7;
  channelLowLimit = 7;
  MenuID = 12;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  ProEnzy1Live();
}
  
void ProEnzy1Live(){ // Affichage valeur et graphique
  lcd.setCursor(6, 0);
  lcd.print("Test Enzyme 1");
  Temp1=(Indio.analogRead(1))*150/100*1.0052+0.7893; // mise à l'échelle et correction
  Temp2=(Indio.analogRead(2))*200/100*1.0065+1.164; // mise à l'échelle et correction
  if ((Temp1 >= 0 and Temp1 < (TempEz1 - Hyst)) or (Temp2 >=0 and Temp2 < (TempEz1 - Hyst))) {
    Indio.digitalWrite(1, HIGH); // Pompe 1
    Indio.digitalWrite(2, LOW); // Pompe 2
  }
  else if (Temp1 > (TempEz1 + Hyst) and Temp2 > TempEz1) {
    Indio.digitalWrite(1, LOW); // Pompe 1
    Indio.digitalWrite(2, LOW); // Pompe 2
  }
  else {
    Indio.digitalWrite(1, LOW); // Pompe 1
    Indio.digitalWrite(2, LOW); // Pompe 2 
  }

  lcd.setCursor(6, 1);
  lcd.print("Set Point");
  lcd.setCursor(67, 1);
  lcd.print(TempEz1);
  lcd.setCursor(120, 1);
  lcd.print(" C ");
  lcd.setCursor(6, 2);
  lcd.print("Temp 1");
  lcd.setCursor(67, 2);
  lcd.print(Temp1);
  lcd.setCursor(120, 2);
  lcd.print(" C ");
  lcd.setCursor(6, 3);
  lcd.print("Temp 2");
  lcd.setCursor(67, 3);
  lcd.print(Temp2);
  lcd.setCursor(120, 3);
  lcd.print(" C ");
  lcd.setCursor(6, 7);
  lcd.print("Back   ");
  
}

  void ProEnzy2() {
  Indio.setADCResolution(14);
  Indio.analogReadMode(1, mA_p);
  Indio.analogReadMode(2, mA_p);
  channel = 7;
  channelUpLimit = 7;
  channelLowLimit = 7;
  MenuID = 19;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  ProEnzy2Live();
}

void ProEnzy2Live(){ // Affichage valeur et graphique
  lcd.setCursor(6, 0);
  lcd.print("Test Enzyme 2");
  Temp1=(Indio.analogRead(1))*150/100*1.0052+0.7893; // mise à l'échelle et correction
  Temp2=(Indio.analogRead(2))*200/100*1.0065+1.164; // mise à l'échelle et correction
  if ((Temp1 >= 0 and Temp1 < (TempEz2 - Hyst)) or (Temp2 >=0 and Temp2 < (TempEz2 - Hyst))) {
    Indio.digitalWrite(1, HIGH); // Pompe 1
    Indio.digitalWrite(2, LOW); // Pompe 2
  }
  else if (Temp1 > (TempEz2 + Hyst) and Temp2 > TempEz2) {
    Indio.digitalWrite(1, LOW); // Pompe 1
    Indio.digitalWrite(2, LOW); // Pompe 2
  }
  else {
    Indio.digitalWrite(1, LOW); // Pompe 1
    Indio.digitalWrite(2, LOW); // Pompe 2 
  }

  lcd.setCursor(6, 1);
  lcd.print("Set Point");
  lcd.setCursor(67, 1);
  lcd.print(TempEz2);
  lcd.setCursor(120, 1);
  lcd.print(" C ");
  lcd.setCursor(6, 2);
  lcd.print("Temp 1");
  lcd.setCursor(67, 2);
  lcd.print(Temp1);
  lcd.setCursor(120, 2);
  lcd.print(" C ");
  lcd.setCursor(6, 3);
  lcd.print("Temp 2");
  lcd.setCursor(67, 3);
  lcd.print(Temp2);
  lcd.setCursor(120, 3);
  lcd.print(" C ");
  lcd.setCursor(6, 7);
  lcd.print("Back   ");
  
}

  void ProInEnz() {
  Indio.setADCResolution(14);
  Indio.analogReadMode(1, mA_p);
  Indio.analogReadMode(2, mA_p);
  channel = 7;
  channelUpLimit = 7;
  channelLowLimit = 7;
  MenuID = 18;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  ProInEnzLive();
}

  void ProInEnzLive(){ // Affichage valeur et graphique
  lcd.setCursor(6, 0);
  lcd.print("Test Inactivation Enz");
  Temp1=(Indio.analogRead(1))*150/100*1.0052+0.7893; // mise à l'échelle et correction
  Temp2=(Indio.analogRead(2))*200/100*1.0065+1.164; // mise à l'échelle et correction
  if ((Temp1 >= 0 and Temp1 < (TempInZ - Hyst)) or (Temp2 >=0 and Temp2 < (TempInZ - Hyst))) {
    Indio.digitalWrite(1, HIGH); // Pompe 1
    Indio.digitalWrite(2, LOW); // Pompe 2
  }
  else if (Temp1 > (TempInZ + Hyst) and Temp2 > TempInZ) {
    Indio.digitalWrite(1, LOW); // Pompe 1
    Indio.digitalWrite(2, LOW); // Pompe 2
  }
  else {
    Indio.digitalWrite(1, LOW); // Pompe 1
    Indio.digitalWrite(2, LOW); // Pompe 2 
  }

  lcd.setCursor(6, 1);
  lcd.print("Set Point");
  lcd.setCursor(67, 1);
  lcd.print(TempInZ);
  lcd.setCursor(120, 1);
  lcd.print(" C ");
  lcd.setCursor(6, 2);
  lcd.print("Temp 1");
  lcd.setCursor(67, 2);
  lcd.print(Temp1);
  lcd.setCursor(120, 2);
  lcd.print(" C ");
  lcd.setCursor(6, 3);
  lcd.print("Temp 2");
  lcd.setCursor(67, 3);
  lcd.print(Temp2);
  lcd.setCursor(120, 3);
  lcd.print(" C ");
  lcd.setCursor(6, 7);
  lcd.print("Back   ");
  
}

void ProForPompe() {

  for (int i = 1; i <= 2; i++) {
    Indio.digitalMode(i, OUTPUT);
    Indio.digitalWrite(i, LOW);
  }
  channel = 0;
  channelUpLimit = 7;
  channelLowLimit = 0;
  MenuID = 17;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Pompe 1");
  lcd.setCursor(6, 1);
  lcd.print("Pompe 2");
  lcd.setCursor(6, 7);
  lcd.print("Back   ");
}

void ProNonUtil() {
  channel = 2;
  channelUpLimit = 2;
  channelLowLimit = 2;
  MenuID = 16;
  MenuLevel = 3;
  enterPressed = 0;
  lcd.clear();
  ScrollCursor();
  lcd.setCursor(6, 0);
  lcd.print("Programme Non Utilise");
  lcd.setCursor(6, 2);
  lcd.print("Back");
}


//---------------------------------------------------------------------------------------------------------------------------------------------------
//UI control logic, please edit this function to reflect the specific menus that your created above and your desired actions for each cursor position
//---------------------------------------------------------------------------------------------------------------------------------------------------



void Navigate()
{

  if (valueEditing != 1) {

    if (MenuLevel == 0) //check if current activated menu is the 'splash screen' (first level)
    {
      {
        if (enterPressed == 1) MenuSelect(); //if enter is pressed load the 'Main menu'
      }
    }

    if (MenuLevel == 1) { //check if current activated menu is the 'Main menu' (first level)
    
      if (channel == 3 && enterPressed == 1) MenuBrassage(); //if cursor is on the first row and enter is pressed load the 'Setup' menu 
      if (channel == 4 && enterPressed == 1) MenuMiseEchelle(); //if cursor is on the second row and enter is pressed load the 'Demo' menu
      if (channel == 2 && enterPressed == 1) MenuWelcome(); //if cursor is on the third row and enter is pressed load the 'splash screen'
    }



    if (MenuLevel == 2) {


       if (MenuID == 3) {
         if (channel == 1 && enterPressed == 1) MenuSelect();
      }
      if (MenuID == 11) {
        if (channel == 0 && enterPressed == 1) ProEmpattage(); //"Empattage"
        if (channel == 1 && enterPressed == 1) ProEnzy1(); //"Enzymatique 1"
        if (channel == 2 && enterPressed == 1) ProEnzy2(); //"Enzymatique 2"
        if (channel == 3 && enterPressed == 1) ProInEnz(); //"Inactivation Ez"
        if (channel == 4 && enterPressed == 1) ProForPompe(); //"Forcage Pompe"
        if (channel == 5 && enterPressed == 1) ProNonUtil(); //"Non utilisé"
        if (channel == 6 && enterPressed == 1) MenuSetup(); //if cursor is on the first row and enter is pressed load the 'Setup' menu
        if (channel == 7 && enterPressed == 1) MenuSelect();
      }
    }

    if (MenuLevel == 3) {


      if (MenuID == 12) {
          if ((millis() - lastLCDredraw) > 268) {
            ProEnzy1();
            lastLCDredraw = millis();
          }

        if (channel == 7 && enterPressed == 1) MenuBrassage();
          }
    
      if (MenuID == 14) {
          if ((millis() - lastLCDredraw) > 268) {
            ProEmpattage();
            lastLCDredraw = millis();
          }

        if (channel == 7 && enterPressed == 1) {
            MenuBrassage();
            Indio.digitalWrite(1, LOW); // Pompe 1
            Indio.digitalWrite(2, LOW); // Pompe 2 
            }
        }

      if (MenuID == 18) {
          if ((millis() - lastLCDredraw) > 268) {
            ProInEnz();
            lastLCDredraw = millis();
          }

        if (channel == 7 && enterPressed == 1) MenuBrassage();
          }
    
      if (MenuID == 19) {
          if ((millis() - lastLCDredraw) > 268) {
            ProEnzy2();
            lastLCDredraw = millis();
          }
      
        if (channel == 7 && enterPressed == 1) MenuBrassage();
          }

      if (MenuID == 9) {
        if (channel == 0 && enterPressed == 1) //using 'value editing mode' to edit a variable using the UI
        {
          TargetValue = backlightIntensity; //copy variable to be edited to 'Target value'
          backlightIntensity = EditValue();
          analogWrite(backlightPin, (map(backlightIntensity, 5, 0, 255, 0)));

        }
        
        if (channel == 1 && enterPressed == 1) MenuParametersReset();

        if (channel == 2 && enterPressed == 1) MenuBrassage();
          }

      if (MenuID == 10) {
        if (channel == 4 && enterPressed == 1) ResetParameters();

        if (channel == 2 && enterPressed == 1) MenuBrassage();
          }

      if (MenuID == 16) {

        if (channel == 2 && enterPressed == 1) {
            MenuBrassage();
        }
      }

      if (MenuID == 17) {

        if ( buttonEnterState == LOW )
        {
          lcd.setCursor(0, channel);
          lcd.print("*");
        }

        if ( buttonEnterState == HIGH )
        {
          lcd.setCursor(0, channel);
          lcd.print(">");
        }

        if (channel == 0 && buttonEnterState == LOW) Indio.digitalWrite(1, HIGH); // Pompe 1
        if (channel == 0 && buttonEnterState == HIGH) Indio.digitalWrite(1, LOW); // Pompe 1
        if (channel == 1 && buttonEnterState == LOW) Indio.digitalWrite(2, HIGH); // Pompe 2
        if (channel == 1 && buttonEnterState == HIGH) Indio.digitalWrite(2, LOW); // Pompe 2
        if (channel == 7 && enterPressed == 1) MenuBrassage(); 
      }
    
    

    }

    //dont remove this part
    if (channel != lastChannel && valueEditing != 1 && MenuID != 0) { //updates the cursor position if button counter changed and 'value editing mode' is not running
      ScrollCursor();
    }
  }
}


//---------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------


float EditValue() //a function to edit a variable using the UI - function is called by the main 'Navigate' UI control function and is loaded with a variable to be edited
{
  row = channel; //save the current cursor position so that after using the buttons for 'value editing mode' the cursor position can be reinstated.
  channel = 0; //reset the button counter so to avoid carrying over a value from the cursor.
  constrainEnc = 0; //disable constrainment of button counter's range
  valueEditingInc = 1; //increment for each button press
  valueEditing = 1; //flag to indicate that we are going into 'value editing mode'.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over
  while (enterPressed != 1) { //stays in 'value editing mode' until enter is pressed
    ReadButtons(); //check the buttons for any change
    lcd.setCursor(0, row);
    lcd.print("*");
    if (channel != lastChannel) { //when up or down button is pressed
      if (channel < lastChannel && TargetValue <= 4) { //if 'Up' button is pressed, and is within constraint range.
        TargetValue += valueEditingInc; //increment target variable with pre-defined increment value
      }
      if (channel > lastChannel && TargetValue > 0) { //if 'Down' button is pressed, and is within constraint range.
        TargetValue -= valueEditingInc ; //decrement target variable with pre-defined increment value
      }
      //clear a section of a row to make space for updated value
      for (int i = 60; i <= 70; i++) {
        lcd.setCursor(i, row);
        lcd.print("   ");
      }
      //print updated value
      lcd.setCursor(66, row);
      Serial.println(TargetValue);
      lcd.print(TargetValue, 0);
      lastChannel = channel;
    }
    //delay(50);
  }
  channel = row; //load back the previous row position to the button counter so that the cursor stays in the same position as it was left before switching to 'value editing mode'
  constrainEnc = 1; //enable constrainment of button counter's range so to stay within the menu's range
  channelUpLimit = 2; //upper row limit
  valueEditing = 0; //flag to indicate that we are leaving 'value editing mode'
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over
  return TargetValue; //return the edited value to the main 'Navigate' UI control function for further processing
}

/* float EditFloatValue() //a function to edit a variable using the UI - function is called by the main 'Navigate' UI control function and is loaded with a variable to be edited
{
  row = channel; //save the current cursor position so that after using the buttons for 'value editing mode' the cursor position can be reinstated.
  channel = 0; //reset the button counter so to avoid carrying over a value from the cursor.
  constrainEnc = 0; //disable constrainment of button counter's range
  valueEditingInc = 0.5; //increment for each button press
  valueEditing = 1; //flag to indicate that we are going into 'value editing mode'.
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over
  while (enterPressed != 1) { //stays in 'value editing mode' until enter is pressed
    ReadButtons(); //check the buttons for any change
    lcd.setCursor(0, row);
    lcd.print("*");
    if (channel != lastChannel) { //when up or down button is pressed
      if (channel < lastChannel && TargetValue <= anOutUpLimit) { //if 'Up' button is pressed, and is within constraint range.
        TargetValue += valueEditingInc; //increment target variable with pre-defined increment value
      }
      if (channel > lastChannel && TargetValue > 0) { //if 'Down' button is pressed, and is within constraint range.
        TargetValue -= valueEditingInc ; //decrement target variable with pre-defined increment value
      }
      //clear a section of a row to make space for updated value
      for (int i = 35; i <= 50; i++) {
        lcd.setCursor(i, row);
        lcd.print("   ");
      }
      //print updated value
      lcd.setCursor(35, row);
      Serial.println(TargetValue);
      lcd.print(TargetValue, 2);
      lastChannel = channel;
    }
    //delay(50);
  }
  channel = row; //load back the previous row position to the button counter so that the cursor stays in the same position as it was left before switching to 'value editing mode'
  constrainEnc = 1; //enable constrainment of button counter's range so to stay within the menu's range
  channelUpLimit = 2; //upper row limit
  valueEditing = 0; //flag to indicate that we are leaving 'value editing mode'
  enterPressed = 0; //clears any possible accidental "Enter" presses that could have been caried over
  return TargetValue; //return the edited value to the main 'Navigate' UI control function for further processing
} */


//---------------------------------------------------------------------------------------------------------------------------------------------
// Peripheral functions
//---------------------------------------------------------------------------------------------------------------------------------------------
void ReadButtons() {

  buttonEnterState = digitalRead(buttonEnterPin);
  buttonUpState = digitalRead(buttonUpPin);
  buttonDownState = digitalRead(buttonDownPin);

  if (buttonEnterState == HIGH && prevBtnEnt == LOW)
  {
    if ((millis() - lastBtnEnt) > transEntInt)
    {
      enterPressed = 1;
    }
    lastBtnEnt = millis();
    lastAdminActionTime = millis();
    Serial.println(enterPressed);
  }
  prevBtnEnt = buttonEnterState;


  if (buttonUpState == HIGH && prevBtnUp == LOW)
  {
    if ((millis() - lastBtnUp) > transInt)
    {
      channel--;
    }
    lastBtnUp = millis();
    lastAdminActionTime = millis();
    //Serial.println("UpPressed");
  }
  prevBtnUp = buttonUpState;


  if (buttonDownState == HIGH && prevBtnDown == LOW)
  {
    if ((millis() - lastBtnDown) > transInt)
    {
      channel++;
    }
    lastBtnDown = millis();
    lastAdminActionTime = millis();
  }
  prevBtnDown = buttonDownState;

  if (constrainEnc == 1) {
    channel = constrain(channel, channelLowLimit, channelUpLimit);
  }

}

void SetInput() { // a simple function called to set a group of pins as inputs
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);
  pinMode(D8, INPUT);
  pinMode(D9, INPUT);
  pinMode(D10, INPUT);
  pinMode(D11, INPUT);
  pinMode(D12, INPUT);
  pinMode(D14, INPUT);
  pinMode(D15, INPUT);
  pinMode(D16, INPUT);
  pinMode(D17, INPUT);
}

void ResetParameters() { //resets the setup parameters of Industruino and saves the settings to EEPROM

  backlightIntensity = backlightIntensityDef; //load the default backlight intensity value
  analogWrite(backlightPin, (map(backlightIntensity, 5, 0, 255, 0))); //map the value (from 0-5) to a corresponding PWM value (0-255) and update the output
  MenuSetup(); //return to the setup menu
}



//---------------------------------------------------------------------------------------------------------------------------------------------
// UI core functions
//---------------------------------------------------------------------------------------------------------------------------------------------



void ScrollCursor() //makes the cursor move
{
  lastChannel = channel; //keep track button counter changes
  for (int i = 0; i <= 6; i++) { //clear the whole column when redrawing a new cursor
    lcd.setCursor(coll, i);
    lcd.print(" ");
  }
  lcd.setCursor(coll, channel); //set new cursor position
  lcd.print(">"); //draw cursor

}
