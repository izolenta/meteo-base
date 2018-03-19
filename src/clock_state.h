#include <inttypes.h>
#include "display_state.h"
#include "weather_data.h"

class ClockState {
private:
    int clockBrightness = 7;
public:
  uint32_t lastTimeSync = 0;
  uint32_t lastTimeSyncAttempt = 0;
  DisplayState displayState;
  WeatherData weatherData;
  int getBrightness();
  void increaseBrightness();
  void decreaseBrightness();
  ClockState();
};

ClockState::ClockState() {
  displayState = DisplayState();
  weatherData = WeatherData();
}

int ClockState::getBrightness() {
  return clockBrightness;
}

void ClockState::increaseBrightness() {
  clockBrightness = min(clockBrightness + 1, 15);
}

void ClockState::decreaseBrightness() {
  clockBrightness = max(clockBrightness - 1, 0);
}