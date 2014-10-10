#include <PS2X_lib.h>  //for v1.6

//-----------     Line Sensor -----------------------------------------------------------
// Light Sensor LSS05
// Connect LSS05 O1-05 to Arduino digital pins
// I use 0-4 on Leonardo

// Initate state of LSS05 outputs
int LSS[] = {
  0, 0, 0, 0, 0};

// Start & End pin for LSS05 O1-O5 on Arduino
int LSSStartPin = 1;
int LSSEndPin = 5;
//---------------------------------------------------------------------------------------

//-----------     Autonomous Stage ------------------------------------------------------
int stage = 0; // 0 - Ready, 1 - Go Pickup Point, 2 - Pick up ball, 3 - Go Dropoff Point, 4 - Dropoff ball, 5 - End
int controlMode = 0; //0 - manual, 1 - auto
//---------------------------------------------------------------------------------------

//-----------     PS2 Controller --------------------------------------------------------
PS2X ps2x; // create PS2 Controller Class

//right now, the library does NOT support hot pluggable controllers, meaning
//you must always either restart your Arduino after you conect the controller,
//or call config_gamepad(pins) again after connecting the controller.
int error = 0;
byte controllerType = 0;
//---------------------------------------------------------------------------------------

//-----------     Pin for Motor  --------------------------------------------------------
int lwf = 6; //arduino pwm 6
int lwb = 8; //arduino pwm 8
int rwf = 7; //arduino pwm 7
int rwb = 9; //arduino pwm 9
int ms = 22; //arduino digital 22
//---------------------------------------------------------------------------------------

void setup(){
  Serial.begin(57600);
  
//-----------     Line Sensor -----------------------------------------------------------
  for(int i = LSSStartPin; i <= LSSEndPin; i ++) {
    pinMode(i, INPUT);
  }
//---------------------------------------------------------------------------------------

//-----------     PS2 Controller --------------------------------------------------------
  pinMode(lwf, OUTPUT);
  pinMode(lwb, OUTPUT);
  pinMode(rwf, OUTPUT);
  pinMode(rwb, OUTPUT);
  pinMode(ms, OUTPUT);
 
  error = ps2x.config_gamepad(13,11,10,12, true, true);   //setup pins and settings:  GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
                                                          // ps2 pin 1-data, 2-command, 3-vibrate, 4-ground, 5-power, 6-attention, 7-clock, 8-???, 9-acknowledge
  controllerType = ps2x.readType();
//---------------------------------------------------------------------------------------
}

void loop() {
  if(error == 1) //skip loop if no controller found
    return;

  //if(controllerType == 1){ //DualShock Controller
    ps2x.read_gamepad();          //read controller

    if(ps2x.ButtonPressed(PSB_START)) {
      stage = 1; //start auto mode
      Serial.println("auto system started");
    }
    if(ps2x.ButtonPressed(PSB_SELECT)) {
      if (controlMode == 0) {
          controlMode = 1; //set mode to auto
          stage = 0;
          Serial.println("mode auto");
      } else {
          controlMode = 0; //set mode to manual
          Serial.println("mode manual");
      }
    }

    if(ps2x.Button(PSB_L2)){
      right_wheel_on();
      Serial.println("Left Forward");
    }
    if(ps2x.Button(PSB_R2)){
      left_wheel_on();
      Serial.println("Right Forward");
    }
    if(ps2x.Button(PSB_PAD_UP)){
      open_storage();
      Serial.println("Up pressed");
    }
    //unused buttons PSB_PAD_LEFT, PSB_PAD_RIGHT, PSB_L3, PSB_R3, PSB_L2, PSB_R2, PSB_GREEN, PSB_RED, PSB_PINK, PSB_BLUE
  //}

  if (controlMode == 1) {
      axReadLSS();
      switch ( stage ) {
        case 1: // go forward till reach pickup point
          if(LSS[0] == 0 && LSS[1] == 1 && LSS[2] == 1 && LSS[3] == 1 && LSS[4] == 0) {  //straight line
             both_wheel_on();
          }
          if(LSS[0] == 1 && LSS[1] == 1 && LSS[2] == 1 && LSS[3] == 1 && LSS[4] == 1) {  //reach crossroad, aka pickup point
             both_wheel_off();
             stage = 2;
          }
        break;
        case 2: //stop at pick up point & pick up ball
          stage = 3;
          delay(3000);
        break;
        case 3: //go forward till reach Dropoff point
          if(LSS[0] == 0 && LSS[1] == 1 && LSS[2] == 1 && LSS[3] == 1 && LSS[4] == 0) {  //straight line
             both_wheel_on();
          }
          
          if(LSS[0] == 0 && LSS[1] == 0 && LSS[2] == 1 && LSS[3] == 1 && LSS[4] == 0) {  //right curve
             left_wheel_on();
          }
          if(LSS[0] == 0 && LSS[1] == 0 && LSS[2] == 0 && LSS[3] == 1 && LSS[4] == 1) {  //extreme right curve
             left_wheel_on();
          }
          
          if(LSS[0] == 1 && LSS[1] == 1 && LSS[2] == 1 && LSS[3] == 1 && LSS[4] == 1) {  //reach crossroad, aka pickup point
             both_wheel_off();
             stage = 4;
          }
        break;
        case 4: //stop at drop point & drop ball
          stage = 5;
          delay(3000);
        break;
        case 5: //End auto, revert to manual mode
          controlMode = 0;
        break;
      }
  }

  delay(50);
}

void left_wheel_on() {
  digitalWrite(lwf, HIGH);
  digitalWrite(rwf, LOW);
}

void right_wheel_on() {
  digitalWrite(lwf, LOW);
  digitalWrite(rwf, HIGH); 
}

void both_wheel_on() {
  digitalWrite(lwf, HIGH);
  digitalWrite(rwf, HIGH); 
}

void both_wheel_off() {
  digitalWrite(lwf, LOW);
  digitalWrite(rwf, LOW); 
}

void open_storage() {
  digitalWrite(ms, LOW);
}

void axReadLSS() {  
  for(int i = LSSStartPin; i <= LSSEndPin; i ++) {
    LSS[i] = digitalRead(i);
  }
}
