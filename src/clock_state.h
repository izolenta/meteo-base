#include <inttypes.h>
#include "display_state.h"
#include "weather_data.h"

class ClockState {
public:
  uint32_t lastTimeSync = 0;
  uint32_t lastTimeSyncAttempt = 0;
  DisplayState displayState;
  WeatherData weatherData;
  ClockState();
};

ClockState::ClockState() {
  displayState = DisplayState();
  weatherData = WeatherData();
}
