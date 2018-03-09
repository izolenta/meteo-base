#include <inttypes.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "time_handle.h"
#include "lang_convert.h"

const String API_KEY = "ce2055f52883208990dca90dc7b07fc4";
const String CITY = "498817";

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
  result+=String(currentTemp);
  result+="\x0a6, ";
  result+=conditions;
  result+=(". Атмосферное давление "+String(pressure)+" мм. рт. ст., относительная влажность "+humidity+"\x025");
  return result;
}
