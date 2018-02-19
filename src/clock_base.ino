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
}

void loop(void)
{
  _checkTime();
  _checkStateMachine();

  P.displayAnimate();

  if (P.getZoneStatus(ZONE_LOWER) && P.getZoneStatus(ZONE_UPPER))
  {
    if (displayState == STATE_DISPLAY_TIME) {
      P.setFont(ZONE_LOWER, BigFontLower);
      P.setFont(ZONE_UPPER, BigFontUpper);
      P.displayZoneText(ZONE_LOWER, currentTimeString, PA_CENTER, 0, 50, PA_PRINT, PA_NO_EFFECT);
      P.displayZoneText(ZONE_UPPER, currentTimeString, PA_CENTER, 0, 50, PA_PRINT, PA_NO_EFFECT);
    }
    else if (displayState == STATE_DISPLAY_TEMPERATURE) {
      P.setFont(ZONE_UPPER, genericFont);
      P.setFont(ZONE_LOWER, genericFont);
      P.displayZoneText(ZONE_UPPER, "-11\x0a6", PA_LEFT, 0, 50, PA_PRINT, PA_NO_EFFECT);
      P.displayZoneText(ZONE_LOWER, "+25\x0a6", PA_LEFT, 0, 50, PA_PRINT, PA_NO_EFFECT);
    }
    else if (displayState == STATE_DISPLAY_DATE) {
      P.setFont(ZONE_UPPER, genericFont);
      P.setFont(ZONE_LOWER, genericFont);
      P.displayZoneText(ZONE_UPPER, "хуй.", PA_LEFT, 0, 50, PA_PRINT, PA_NO_EFFECT);
      P.displayZoneText(ZONE_LOWER, "пысько", PA_LEFT, 0, 50, PA_PRINT, PA_NO_EFFECT);
    }
    // synchronise the start
    P.synchZoneStart();
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
    if (displayState == STATE_DISPLAY_TIME) {
        displayState = STATE_DISPLAY_TEMPERATURE;
    }
    else {
      displayState = STATE_DISPLAY_TIME;
    }
  }
  else if (b1 == LONG_PRESS) {
    if (displayState == STATE_DISPLAY_TIME) {
        displayState = STATE_DISPLAY_DATE;
    }
  }
}

int _checkButtonPress(ButtonTimeDescription &btn) {
  int state = digitalRead(btn.buttonPin);
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
      long endPressed = millis();
      long timeHold = endPressed - btn.startPressed;
      if (timeHold < 200) {
        return NO_RESULT;
      }
      btn.buttonState = state;
      if (timeHold >= 1000) {
        return LONG_PRESS;
      }
      return SHORT_PRESS;
    }
  }
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
//    printDateTime(now1);
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

// #define countof(a) (sizeof(a) / sizeof(a[0]))
//
// void printDateTime(const RtcDateTime& dt)
// {
//     char datestring[20];
//
//     snprintf_P(datestring,
//             countof(datestring),
//             PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
//             dt.Month(),
//             dt.Day(),
//             dt.Year(),
//             dt.Hour(),
//             dt.Minute(),
//             dt.Second() );
//     Serial.print(datestring);
// }
