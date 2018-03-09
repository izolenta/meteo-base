#pragma once

#include <Wire.h>
#include <RtcDS3231.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#define NTP_PACKET_SIZE 48
#define timeZone 3
const IPAddress timeServer(132, 163, 96, 1);

RtcDS3231<TwoWire> Rtc(Wire);
WiFiUDP Udp;

class TimeHandle {
private:
  byte packetBuffer[NTP_PACKET_SIZE];
  void sendNTPpacket(IPAddress address);
  time_t getNtpTime();
public:
  TimeHandle();
  void setupRtc();
  bool trySyncRtc();
  RtcDateTime getCurrentTime();
};

TimeHandle::TimeHandle() {
}
RtcDateTime TimeHandle::getCurrentTime() {
  return Rtc.GetDateTime();
}

void TimeHandle::setupRtc() {
  Rtc.Begin();
  if (!Rtc.GetIsRunning())
  {
      Serial.println("RTC was not actively running, starting now");
      Rtc.SetIsRunning(true);
  }
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}

bool TimeHandle::trySyncRtc() {
  Serial.println("trying for sync");
  int attempts = 3;
  while (attempts > 0) {
    time_t res = getNtpTime();
    if (res != 0) {
      RtcDateTime current = RtcDateTime(year(res), month(res), day(res), hour(res), minute(res), second(res));
      Rtc.SetDateTime(current);
      RtcDateTime now1 = Rtc.GetDateTime();
      Serial.println();
      Serial.println("seems synced!");
      return true;
    }
    attempts--;
  }
  RtcDateTime now2 = Rtc.GetDateTime();
  Serial.println();
  Serial.println("sync failed");
  return false;
}

// send an NTP request to the time server at the given address
void TimeHandle::sendNTPpacket(IPAddress address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

time_t TimeHandle::getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}
