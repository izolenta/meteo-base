#include <inttypes.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include "time_handle.h"

const String API_KEY = "ce2055f52883208990dca90dc7b07fc4";
const String CITY = "498817";

class WeatherData {
private:
  int currentTemp;
  String conditions;
  int pressure;
  int humidity;
  int windSpeed;
  int windDegree;
  uint32_t sunset;
  uint32_t sunrise;
public:
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
  String url = "http://api.openweathermap.org/data/2.5/weather?id="+CITY+"&appid="+API_KEY+"&lang=ru&units=metric";
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
    }

    http.end();
  }
}

String WeatherData::getWeatherString() {
  return "aaa";
}
