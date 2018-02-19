#include "clock_base.h"

void setup(void)
{
  Serial.begin(115200);

  // initialise the LED display
  P.begin(MAX_ZONES);

  // Set up zones for 2 halves of the display
  // Each zone gets a different font, making up the top
  // and bottom half of each letter
  P.setZone(ZONE_LOWER, 0, ZONE_SIZE - 1);
  P.setFont(ZONE_LOWER, BigFontLower);

  P.setZone(ZONE_UPPER, ZONE_SIZE, MAX_DEVICES-1);
  P.setFont(ZONE_UPPER, BigFontUpper);
  P.setCharSpacing(1);
  P.setZoneEffect(ZONE_UPPER, true, PA_FLIP_UD);
  P.setZoneEffect(ZONE_UPPER, true, PA_FLIP_LR);
  P.setIntensity(4);

  pinMode(BUTTON1_PIN, INPUT);

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

  Wire.begin(4, 5);

  _setupRTC();
  _trySyncRTC();

  displayState.requestChangeState(STATE_DISPLAY_TIME);
}

void loop(void)
{
  _checkTime();
  _checkStateMachine();

  P.displayAnimate();

  bool upperAnimationComplete = P.getZoneStatus(ZONE_UPPER);
  bool lowerAnimationComplete = P.getZoneStatus(ZONE_LOWER);
  bool allAnimationComplete = upperAnimationComplete && lowerAnimationComplete;
  bool someAnimationComplete = upperAnimationComplete || lowerAnimationComplete;
  bool stateChange = displayState.stateChangeRequested();

  if (stateChange || someAnimationComplete) {
    if (stateChange) {
      P.displayClear();
    }
    Serial.println("state change requested to" + String(displayState.getRequestedState()));
    if (displayState.getRequestedState() == STATE_DISPLAY_TIME) {
      _displayBigClock();
    }
    else if (displayState.getRequestedState() == STATE_DISPLAY_TEMPERATURE) {
      if (stateChange) {
        P.setFont(ZONE_UPPER, genericFont);
        P.setFont(ZONE_LOWER, genericFont);
        P.displayZoneText(ZONE_UPPER, "-11\x0a6", PA_LEFT, 0, 4000, PA_PRINT, PA_NO_EFFECT);
        P.displayZoneText(ZONE_LOWER, "+25\x0a6", PA_LEFT, 0, 4000, PA_PRINT, PA_NO_EFFECT);
      }
      else if (allAnimationComplete) {
        displayState.requestChangeState(STATE_DISPLAY_TIME);
      }
    }
    else if (displayState.getRequestedState() == STATE_DISPLAY_DATE) {
      if (stateChange) {
        _displayUpperClock();
        _displayLowerScroll(_constructDateString());
      }
      else if (upperAnimationComplete) {
        _displayUpperClock();
      }
      else if (lowerAnimationComplete) {
        displayState.requestChangeState(STATE_DISPLAY_TIME);
      }
    }
    displayState.commitStateChange();
  }
}

void _checkTime() {
  time_t nowTime = now();
  if (nowTime > previousTime) {
    RtcDateTime curr = Rtc.GetDateTime();
    hours = curr.Hour();
    minutes = curr.Minute();
    seconds = curr.Second();
    String result = _get2digits(hours) + delim[second(nowTime) % 2] + _get2digits(minutes);
    result.toCharArray(currentTimeString, 6);
    previousTime = nowTime;
  }
}

void _checkStateMachine() {
  int b1 = _checkButtonPress(button1);
  if (b1 == SHORT_PRESS) {
    if (displayState.getCurrentState() == STATE_DISPLAY_TIME) {
      displayState.requestChangeState(STATE_DISPLAY_TEMPERATURE);
    }
    else {
      displayState.requestChangeState(STATE_DISPLAY_TIME);
    }
  }
  else if (b1 == LONG_PRESS) {
    if (displayState.getCurrentState() == STATE_DISPLAY_TIME) {
      displayState.requestChangeState(STATE_DISPLAY_DATE);
    }
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
  String result = String(curr.Day()) + " " + monthNames[curr.Month()-1] + " " + String(curr.Year()) + ", "+dayOfWeekNames[curr.DayOfWeek()-1];
  return result;
}

int _checkButtonPress(ButtonTimeDescription &btn) {

  int state = digitalRead(btn.buttonPin);
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

void _setupRTC() {
  Rtc.Begin();
  if (!Rtc.GetIsRunning())
  {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

void _trySyncRTC() {
  Serial.println("trying for sync");
  time_t res = getNtpTime(Udp);
  if (res != 0) {
    RtcDateTime current = RtcDateTime(year(res), month(res), day(res), hour(res), minute(res), second(res));
    Rtc.SetDateTime(current);
    RtcDateTime now1 = Rtc.GetDateTime();
    Serial.println();
    Serial.println("seems synced!");
  }
}

String _get2digits(int number) {
  String res = "";
  if (number >= 0 && number < 10) {
    res = "0";
  }
  res = res + number;
  return res;
}
