#define STATE_INITIAL -1
#define STATE_DISPLAY_TIME 0
#define STATE_DISPLAY_TEMPERATURE 1
#define STATE_DISPLAY_DATE 2
#define STATE_DISPLAY_CURRENT_WEATHER 3
#define STATE_DECREASE_ALARM_ENCODER 4
#define STATE_INCREASE_ALARM_ENCODER 5

class DisplayState {
private:
  int currentState;
  int requestedState;
public:
  DisplayState();
  void requestChangeState(int requestedState);
  bool stateChangeRequested();
  int getCurrentState();
  int getRequestedState();
  void commitStateChange();
};

DisplayState::DisplayState() {
  currentState = STATE_INITIAL;
  requestedState = STATE_INITIAL;
}

void DisplayState::requestChangeState(int requested) {
  if (requestedState != requested && currentState != requested) {
    requestedState = requested;
  }
}

bool DisplayState::stateChangeRequested() {
  return requestedState != currentState;
}

int DisplayState::getCurrentState() {
  return currentState;
}

int DisplayState::getRequestedState() {
  return requestedState;
}

void DisplayState::commitStateChange() {
  if (requestedState == STATE_DECREASE_ALARM_ENCODER || requestedState == STATE_INCREASE_ALARM_ENCODER) {
    requestedState = currentState;
  }
  else {
    currentState = requestedState;
  }
}
  
