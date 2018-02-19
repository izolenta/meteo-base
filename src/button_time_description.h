class ButtonTimeDescription {
public:
  ButtonTimeDescription(int pinNum);
  int buttonState;     // current state of the button
  long startPressed;    // the time button was pressed
  int buttonPin;
  int buttonGlitchThreshold;

};

ButtonTimeDescription::ButtonTimeDescription(int pinNum) {
  buttonPin = pinNum;
  buttonState = 0;
  buttonGlitchThreshold = 0;
  startPressed = 0;
}

