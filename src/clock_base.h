#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);

#include "Parola_Fonts_data.h"
#include "ntp_sync.h"
#include "button_time_description.h"

#define STATE_DISPLAY_TIME 0
#define STATE_DISPLAY_TEMPERATURE 1
#define STATE_DISPLAY_DATE 2

#define MAX_ZONES 2
#define ZONE_SIZE 4
#define MAX_DEVICES (MAX_ZONES * ZONE_SIZE)
#define SCROLL_SPEED  30
#define ZONE_UPPER  1
#define ZONE_LOWER  0

#define CS_PIN    15
#define BUTTON1_PIN  16

#define LONG_PRESS 1
#define SHORT_PRESS 2
#define NO_RESULT 0

#define MAX_BUTTON_GLITCH_THRESHOLD 20

const char ssid[] = "ESP8266";  //  your network SSID (name)
const char pass[] = "espnodeMcu^guestPass";       // your network password
const char delim[] = ":\x080";

const String monthNames[12] = {"������", "�������", "�����", "������", "���", "����", "����", "�������", "��������", "�������", "������", "�������"};
const String dayOfWeekNames[12] = {"�����������", "�������", "�����", "�������", "�������", "�������", "�����������"};

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
MD_Parola P = MD_Parola(CS_PIN, MAX_DEVICES);

int hours = 0;
int minutes = 0;
int seconds = 0;

int displayState = STATE_DISPLAY_TIME;

time_t previousTime = 0;
char currentTimeString[6] = {0};

ButtonTimeDescription button1 = ButtonTimeDescription(BUTTON1_PIN);
