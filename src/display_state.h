#define STATE_INITIAL -1
#define STATE_DISPLAY_TIME 0
#define STATE_DISPLAY_TEMPERATURE 1
#define STATE_DISPLAY_DATE 2

class DisplayState {
private:
  int currentState;
  int requestedState;
public:
  DisplayState(int initialState);
  void requestChangeState(int requestedState);
  bool stateChangeRequested();
  int getCurrentState();
  int getRequestedState();
  void commitStateChange();
};
 
DisplayState::DisplayState(int initial) {
  currentState = initial;
  requestedState = initial;
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
  currentState = requestedState;
}
