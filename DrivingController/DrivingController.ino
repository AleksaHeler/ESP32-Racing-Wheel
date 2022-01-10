#define BOUNCE_WITH_PROMPT_DETECTION    // Make button state changes available immediately

#include <Bounce2.h>      // https://github.com/thomasfredericks/Bounce2
#include <BleGamepad.h> 

#define numOfButtons        8
#define numOfHatSwitches    0
#define enableX             false
#define enableY             false
#define enableZ             false
#define enableRZ            false
#define enableRX            false
#define enableRY            false
#define enableSlider1       true
#define enableSlider2       true
#define enableRudder        false
#define enableThrottle      false
#define enableAccelerator   true
#define enableBrake         true
#define enableSteering      true

#define ACC_PIN       32
#define BRAKE_PIN     33
#define STEER_PIN     34

#define LS_PIN        36  // slider1
#define RS_PIN        39  // slider2

#define BTN1_PIN      18
#define BTN2_PIN      19
#define BTN3_PIN      21
#define BTN4_PIN      22
#define BTN5_PIN      23
#define BTN6_PIN      25
#define BTN7_PIN      26
#define BTN8_PIN      27
Bounce debouncers[numOfButtons];

#define STEER_ZERO_OFFSET     4800
byte buttonPins[numOfButtons] = { 23, 22, 25, 18, 26, 21, 27, 19};
byte physicalButtons[numOfButtons] = { 1, 2, 3, 4, 5, 6, 7, 8 };

BleGamepad bleGamepad("Driving Controller", "aleksaheler", 100);

void setup() 
{
  Serial.begin(115200);
  Serial.println("Starting BLE work!");
  
  //Setup controller
  bleGamepad.setAutoReport(false);
  bleGamepad.setControllerType(CONTROLLER_TYPE_GAMEPAD);  //CONTROLLER_TYPE_JOYSTICK, CONTROLLER_TYPE_GAMEPAD (DEFAULT), CONTROLLER_TYPE_MULTI_AXIS
  bleGamepad.begin(numOfButtons,numOfHatSwitches,enableX,enableY,enableZ,enableRZ,enableRX,enableRY,enableSlider1,enableSlider2,enableRudder,enableThrottle,enableAccelerator,enableBrake,enableSteering);
  
  //Set accelerator and brake to min
  bleGamepad.setAccelerator(-32767);
  bleGamepad.setBrake(-32767);
  //Set steering to center
  bleGamepad.setSteering(0);

  // Set up buttons
  for (byte currentPinIndex = 0 ; currentPinIndex < numOfButtons ; currentPinIndex++)
  {
    pinMode(buttonPins[currentPinIndex], INPUT_PULLUP);
    
    debouncers[currentPinIndex] = Bounce();
    debouncers[currentPinIndex].attach(buttonPins[currentPinIndex]);      // After setting up the button, setup the Bounce instance :
    debouncers[currentPinIndex].interval(5);        
  }
}

void loop() 
{
  if(bleGamepad.isConnected()) 
  {
    // Get input
    double acc = analogRead(ACC_PIN);
    double brake = analogRead(BRAKE_PIN);
    double steer = analogRead(STEER_PIN);
    double ls = analogRead(LS_PIN); // slider1
    double rs = analogRead(RS_PIN); // slider2

    // Calculate output (min -32767  max 32768)
    double acc_val = map(acc, 500, 4095, 32767, -32767); // measure min, measure max, output min, output max
    double brake_val = map(brake, 700, 4095, -32767, 32767);
    double steer_val = map(steer + 300, 100, 4000, -32767, 32767); // + offset
    double ls_val = map(ls, 0, 4095, -32767, 32767);
    double rs_val = map(rs, 0, 4095, -32767, 32767);

    // Clamp values
    if(acc_val < -32767) acc_val = -32767;
    if(acc_val > 32767) acc_val = 32767;
    
    if(brake_val < -32767) brake_val = -32767;
    if(brake_val > 32767) brake_val = 32767;
    
    if(steer_val < -32767) steer_val = -32767;
    if(steer_val > 32767) steer_val = 32767;

    // Apply values
    bleGamepad.setAccelerator(acc_val);
    bleGamepad.setBrake(brake_val);
    bleGamepad.setSteering(steer_val);
    bleGamepad.setSliders(ls_val, rs_val);

    // Do buttons
    for (byte currentIndex = 0 ; currentIndex < numOfButtons ; currentIndex++)
    {
      debouncers[currentIndex].update();

      if (debouncers[currentIndex].fell())
      {
        bleGamepad.press(physicalButtons[currentIndex]);
      }
      else if (debouncers[currentIndex].rose())
      {
        bleGamepad.release(physicalButtons[currentIndex]);
      }
    }
    
    bleGamepad.sendReport();
    delayMicroseconds(50);
  }
}
