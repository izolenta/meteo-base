#pragma once

#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#include <pcf8574_esp.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#include "time_handle.h"
#include "Parola_Fonts_data.h"
#include "button_time_description.h"
#include "clock_state.h"

#define MAX_ZONES 2
#define ZONE_SIZE 4
#define MAX_DEVICES (MAX_ZONES * ZONE_SIZE)
#define SCROLL_SPEED  30
#define ZONE_UPPER  1
#define ZONE_LOWER  0

#define CS_PIN    15
#define BUTTON1_PIN  1
#define BUTTON2_PIN  0
#define ENCODER_A_PIN  2
#define ENCODER_B_PIN  3
#define BUTTON_ENCODER_PIN  4

#define LONG_PRESS 1
#define SHORT_PRESS 2
#define NO_RESULT 0

#define MAX_BUTTON_GLITCH_THRESHOLD 20

const char ssid[] = "ESP8266";  //  your network SSID (name)
const char pass[] = "espnodeMcu^guestPass";       // your network password
const char delim[] = ":\x080";

const String monthNames[12] = {"������", "�������", "�����", "������", "���", "����", "����", "�������", "��������", "�������", "������", "�������"};
const String dayOfWeekNames[7] = {"�����������", "�����������", "�������", "�����", "�������", "�������", "�������"};

unsigned int localPort = 8888;  // local port to listen for UDP packets
MD_Parola P = MD_Parola(CS_PIN, MAX_DEVICES);

int hours = 0;
int minutes = 0;
int seconds = 0;

int homeTemp = 0;
int homePressure = 0;

int encoder_a_state = 0;
long unsigned int encoder_next_check = 0;

ClockState clockState = ClockState();
TimeHandle timeHandle = TimeHandle();

time_t previousTime = 0;
char currentTimeString[6] = {0};

char* scrollMessage = (char*)malloc(1024);
char* homeTemperatureMessage = (char*)malloc(8);
char* awayTemperatureMessage = (char*)malloc(8);

ButtonTimeDescription button1 = ButtonTimeDescription(BUTTON1_PIN);
ButtonTimeDescription button2 = ButtonTimeDescription(BUTTON2_PIN);
ButtonTimeDescription buttonEncoder = ButtonTimeDescription(BUTTON_ENCODER_PIN);

PCF857x pcf8574(0x20, &Wire);
Adafruit_BMP280 bme;
