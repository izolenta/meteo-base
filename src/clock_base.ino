#include "clock_base.h"

void setup(void)
{

  Serial.begin(115200);
  Wire.begin(4, 5);
  if (!bme.begin(0x76)) {
    Serial.println("BME SHIT happens");
  }
  else {
    Serial.println("BME OK");
  }
  pcf8574.begin();

  P.begin(MAX_ZONES);

  P.setZone(ZONE_LOWER, 0, ZONE_SIZE - 1);
  P.setFont(ZONE_LOWER, BigFontLower);

  P.setZone(ZONE_UPPER, ZONE_SIZE, MAX_DEVICES-1);
  P.setFont(ZONE_UPPER, BigFontUpper);
  P.setCharSpacing(1);
  P.setZoneEffect(ZONE_UPPER, true, PA_FLIP_UD);
  P.setZoneEffect(ZONE_UPPER, true, PA_FLIP_LR);
  P.setIntensity(4);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());

  timeHandle.setupRtc();

  _trySyncRTC();

  clockState.displayState.requestChangeState(STATE_DISPLAY_TIME);
}

void loop(void)
{
  _checkTime();
  _checkStateMachine();
  _checkLocalWeatherSensor();

  P.displayAnimate();

  bool upperAnimationComplete = P.getZoneStatus(ZONE_UPPER);
  bool lowerAnimationComplete = P.getZoneStatus(ZONE_LOWER);
  bool allAnimationComplete = upperAnimationComplete && lowerAnimationComplete;
  bool someAnimationComplete = upperAnimationComplete || lowerAnimationComplete;
  bool stateChange = clockState.displayState.stateChangeRequested();

  if (stateChange || someAnimationComplete) {
    if (stateChange) {
      P.displayClear();
    }
    if (clockState.displayState.getRequestedState() == STATE_INCREASE_ALARM_ENCODER) {
      clockState.increaseBrightness();
      P.setIntensity(clockState.getBrightness());
      P.displayAnimate();
    }
    if (clockState.displayState.getRequestedState() == STATE_DECREASE_ALARM_ENCODER) {
      clockState.decreaseBrightness();
      P.setIntensity(clockState.getBrightness());
    }
    if (clockState.displayState.getRequestedState() == STATE_DISPLAY_TIME) {
      _displayBigClock();
    }
    else if (clockState.displayState.getRequestedState() == STATE_DISPLAY_TEMPERATURE) {
      if (stateChange) {
        P.setFont(ZONE_UPPER, genericFont);
        P.setFont(ZONE_LOWER, genericFont);
        String homeTempString = String(homeTemp) + "\x0a6";
        if (homeTemp > 0) {
          homeTempString = "+"+homeTempString;
        }
        int awayTemp = clockState.weatherData.getAwayTemperature();
        String awayTempString = String(awayTemp) + "\x0a6";
        if (awayTemp > 0) {
          awayTempString = "+"+awayTempString;
        }
        homeTempString.toCharArray(homeTemperatureMessage, homeTempString.length()+1);
        awayTempString.toCharArray(awayTemperatureMessage, awayTempString.length()+1);

        P.displayZoneText(ZONE_UPPER, awayTemperatureMessage, PA_LEFT, 0, 4000, PA_PRINT, PA_NO_EFFECT);
        P.displayZoneText(ZONE_LOWER, homeTemperatureMessage, PA_LEFT, 0, 4000, PA_PRINT, PA_NO_EFFECT);
      }
      else if (allAnimationComplete) {
        clockState.displayState.requestChangeState(STATE_DISPLAY_TIME);
      }
    }
    else if (clockState.displayState.getRequestedState() == STATE_DISPLAY_DATE) {
      if (stateChange) {
        _displayUpperClock();
        _displayLowerScroll(_constructDateString());
      }
      else if (upperAnimationComplete) {
        _displayUpperClock();
      }
      else if (lowerAnimationComplete) {
        clockState.displayState.requestChangeState(STATE_DISPLAY_TIME);
      }
    }
    else if (clockState.displayState.getRequestedState() == STATE_DISPLAY_CURRENT_WEATHER) {
      if (stateChange) {
        _displayUpperClock();
        _displayLowerScroll(clockState.weatherData.getWeatherString());
      }
      else if (upperAnimationComplete) {
        _displayUpperClock();
      }
      else if (lowerAnimationComplete) {
        clockState.displayState.requestChangeState(STATE_DISPLAY_TIME);
      }
    }

    clockState.displayState.commitStateChange();
  }
}

void _checkLocalWeatherSensor() {
   homeTemp = floor(bme.readTemperature());
   homePressure = floor(bme.readPressure());
}

void _checkTime() {
  time_t nowTime = now();
  if (nowTime > previousTime) {
    RtcDateTime curr = timeHandle.getCurrentTime();
    hours = curr.Hour();
    minutes = curr.Minute();
    seconds = curr.Second();
    String result = _get2digits(hours) + delim[second(nowTime) % 2] + _get2digits(minutes);
    result.toCharArray(currentTimeString, 6);
    previousTime = nowTime;
    if ((curr.Second() == 15 || curr.Second() == 45) && clockState.displayState.getCurrentState() == STATE_DISPLAY_TIME) {
      clockState.displayState.requestChangeState(STATE_DISPLAY_TEMPERATURE);
    }
    if (curr.TotalSeconds() - clockState.lastTimeSyncAttempt > 3600) {
      _trySyncRTC();
    }
    if (curr.TotalSeconds() - clockState.weatherData.lastWeatherSyncAttempt > 600) {
      _trySyncWeather();
    }
  }
}

void _trySyncWeather() {
  if (clockState.weatherData.updateWeather()) {
    clockState.weatherData.lastWeatherTodaySync = timeHandle.getCurrentTime().TotalSeconds();
  }
  clockState.weatherData.lastWeatherSyncAttempt = timeHandle.getCurrentTime().TotalSeconds();
}

void _checkStateMachine() {
  _checkButtons();
  _checkEncoder();
}

void _checkEncoder() {
  if (millis() >= encoder_next_check) {
    int state = pcf8574.read(ENCODER_A_PIN);
    int direction = pcf8574.read(ENCODER_B_PIN);
    if (state == LOW && encoder_a_state == HIGH) {
      if (direction == HIGH) {
        clockState.displayState.requestChangeState(STATE_INCREASE_ALARM_ENCODER);
      }
      else {
        clockState.displayState.requestChangeState(STATE_DECREASE_ALARM_ENCODER);
      }
    }
    encoder_a_state = state;
    encoder_next_check = millis() + 5;
  }
}

void _checkButtons() {
  int b1 = _checkButtonPress(button1);
  if (b1 == SHORT_PRESS) {
    if (clockState.displayState.getCurrentState() == STATE_DISPLAY_TIME) {
      clockState.displayState.requestChangeState(STATE_DISPLAY_TEMPERATURE);
    }
    else {
      clockState.displayState.requestChangeState(STATE_DISPLAY_TIME);
    }
    return;
  }
  else if (b1 == LONG_PRESS) {
    if (clockState.displayState.getCurrentState() == STATE_DISPLAY_TIME) {
      clockState.displayState.requestChangeState(STATE_DISPLAY_DATE);
    }
    return;
  }
  int b2 = _checkButtonPress(button2);
  if (b2 == SHORT_PRESS) {
    clockState.displayState.requestChangeState(STATE_DISPLAY_CURRENT_WEATHER);
    return;
  }
  else if (b2 == LONG_PRESS) {
    if (clockState.displayState.getCurrentState() == STATE_DISPLAY_TIME) {
      clockState.displayState.requestChangeState(STATE_DISPLAY_DATE);
    }
    return;
  }
  int bEnc = _checkButtonPress(buttonEncoder, true);
  if (bEnc == SHORT_PRESS) {
    clockState.displayState.requestChangeState(STATE_DISPLAY_CURRENT_WEATHER);
    return;
  }
  else if (bEnc == LONG_PRESS) {
    if (clockState.displayState.getCurrentState() == STATE_DISPLAY_TIME) {
      clockState.displayState.requestChangeState(STATE_DISPLAY_DATE);
    }
    return;
  }
}

void _displayBigClock() {
  P.setFont(ZONE_LOWER, BigFontLower);
  P.setFont(ZONE_UPPER, BigFontUpper);
  P.displayZoneText(ZONE_LOWER, currentTimeString, PA_CENTER, 0, 500, PA_PRINT, PA_NO_EFFECT);
  P.displayZoneText(ZONE_UPPER, currentTimeString, PA_CENTER, 0, 500, PA_PRINT, PA_NO_EFFECT);
  P.synchZoneStart();
}

void _displayUpperClock() {
  P.setFont(ZONE_UPPER, genericFont);
  P.displayZoneText(ZONE_UPPER, currentTimeString, PA_CENTER, 0, 500, PA_PRINT, PA_NO_EFFECT);
}

void _displayLowerScroll(String what) {
  what += "        ";
  Serial.println(what);
  Serial.println(what.length());
  P.setFont(ZONE_LOWER, genericFont);
  what.toCharArray(scrollMessage, what.length()+1);
  P.displayZoneText(ZONE_LOWER, scrollMessage, PA_LEFT, 30, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
}

String _constructDateString() {
  RtcDateTime curr = Rtc.GetDateTime();
  Serial.println(curr.Day());
  Serial.println(curr.Month());
  Serial.println(curr.Year());
  Serial.println(curr.DayOfWeek());
  String result = String(curr.Day()) + " " + monthNames[curr.Month()-1] + " " + String(curr.Year()) + ", "+dayOfWeekNames[curr.DayOfWeek()];
  return result;
}

int _checkButtonPress(ButtonTimeDescription &btn) {
  return _checkButtonPress(btn, false);
}

int _checkButtonPress(ButtonTimeDescription &btn, bool inverted) {

  int state = pcf8574.read(btn.buttonPin);
  if (inverted) {
    state = 1 - state;
  }

  long endPressed = millis();
  long timeHold = endPressed - btn.startPressed;

  if (btn.buttonState != state) {
    btn.buttonGlitchThreshold++;
    if (btn.buttonGlitchThreshold < MAX_BUTTON_GLITCH_THRESHOLD) {
      return NO_RESULT;
    }
    btn.buttonGlitchThreshold = 0;
    if (state == HIGH) {
      btn.startPressed = millis();
      btn.buttonState = state;
      return NO_RESULT;
    }
    else {
      if (timeHold < 0) {
        btn.buttonState = state; //long press recovery
        return NO_RESULT;
      }
      else if (timeHold < 200) {
        return NO_RESULT;
      }
      btn.buttonState = state;
      return SHORT_PRESS;
    }
  }
  else if (btn.buttonState == HIGH && timeHold >= 1000) {
    btn.startPressed = millis()+99999999999;
    return LONG_PRESS;
  }
  return NO_RESULT;
}

void _trySyncRTC() {
  if (timeHandle.trySyncRtc()) {
    clockState.lastTimeSync = timeHandle.getCurrentTime().TotalSeconds();
  }
  clockState.lastTimeSyncAttempt = timeHandle.getCurrentTime().TotalSeconds();
}

String _get2digits(int number) {
  String res = "";
  if (number >= 0 && number < 10) {
    res = "0";
  }
  res = res + number;
  return res;
}
