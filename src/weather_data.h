#include <inttypes.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "time_handle.h"
#include "lang_convert.h"

const String API_KEY = "ce2055f52883208990dca90dc7b07fc4";
const String CITY = "493316";
const String windDirections[8] = {"северо-восточный", "восточный", "юго-восточный", "южный", "юго-западный", "западный", "северо-западный", "северный"};

class WeatherData {
private:
  int currentTemp = -99;
  String conditions;
  int pressure;
  int humidity;
  int windSpeed;
  int windDegree;
  uint32_t sunset;
  uint32_t sunrise;
  void parseData(String payload);
  String getWindDirection();
public:
  int getAwayTemperature();
  uint32_t lastWeatherTodaySync = 0;
  uint32_t lastWeatherForecastSync = 0;
  uint32_t lastWeatherSyncAttempt = 0;
  WeatherData();
  bool updateWeather();
  String getWeatherString();
};

WeatherData::WeatherData() {
}

bool WeatherData::updateWeather() {
  bool result = false;
  String url = "http://api.openweathermap.org/data/2.5/weather?id="+CITY+"&appid="+API_KEY+"&lang=ru&units=metric";
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      parseData(payload);
      result = true;
    }
    http.end();
  }
}

int WeatherData::getAwayTemperature() {
  return currentTemp;
}

void WeatherData::parseData(String payload) {
  StaticJsonBuffer<2048> jsonBuffer;

  JsonObject &root = jsonBuffer.parseObject(payload);

  if (!root.success()) {
    Serial.println(F("Failed to parse weather file"));
  }
  else {
    currentTemp = root["main"]["temp"];
    const char *cond = root["weather"][0]["description"];
    char *w1251 = (char*)malloc(256);
    convert_utf8_to_windows1251(cond, w1251, 256);
    conditions = String(w1251);
    free(w1251);
    int tmpPressure = root["main"]["pressure"];
    pressure = floor(tmpPressure * 0.75);
    humidity = root["main"]["humidity"];
    windSpeed = root["wind"]["speed"];
    windDegree = root["wind"]["deg"];
  }
}

String WeatherData::getWeatherString() {
  if (currentTemp < -90) {
    return "Сейчас: нет данных";
  }
  String result = "Сейчас: ";
  if (currentTemp > 0) {
    result+="+";
  }
  result += String(currentTemp);
  result += "\x0a6, ";
  result += conditions;
  result += (". Давление "+String(pressure)+" мм. рт. ст., влажность "+humidity+"\x025. ");
  if (windSpeed < 1) {
    result += "Ветра нет";
  }
  else {
    result += "Ветер ";
    result += getWindDirection();
    result += (", "+String(windSpeed)+" м/с");
  }
  return result;
}

String WeatherData::getWindDirection() {
  Serial.println(windDegree);
  float temp = windDegree - 22.5;
  if (temp < 0) {
    temp+=360;
  }
  int index = floor(temp/45);
  Serial.println(index);
  return windDirections[index];
}
