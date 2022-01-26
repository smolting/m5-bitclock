#include "M5CoreInk.h"
#include "esp_adc_cal.h"
#include "fontExtension.h"
#include "icon.h"
#include "preferences.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <WiFi.h>
#include <driver/rtc_io.h>
#include <Preferences.h>

enum AppStateSource { NEW_STATE, PREFERENCES };

struct AppStateStruct {
  AppStateSource source;
  char price[11];
  char chainTip[8];
  char mempoolSize[5];
  char nextBlockFee[5];

  char date[10];
  char time[10];

  int64_t lastNTPUpdate;
  int64_t lastMetricsUpdate;
  int64_t lastTimeUpdate;
} DefaultAppState = {NEW_STATE, "", "", "", "", "", "", 0, 0, 0};

typedef struct AppStateStruct AppState;

Ink_Sprite InkPageSprite(&M5.M5Ink);
Preferences preferences;

void convertRTCToCTm(RTC rtc, struct tm *ctime) {
  Serial.println("\nconvertRTCtoCTm");
  RTC_DateTypeDef rtcDate;
  RTC_TimeTypeDef rtcTime;

  Serial.println("\n   rtc GetDate");
  rtc.GetDate(&rtcDate);
  Serial.printf("%i-%i-%i\n", rtcDate.Year, rtcDate.Month - 1, rtcDate.Date);

  Serial.print("\n   rtc GetTime");
  rtc.GetTime(&rtcTime);
  Serial.printf("\n   Hours: %i", rtcTime.Hours);
  Serial.printf("\n   Minutes: %i", rtcTime.Minutes);
  delay(500);
  ctime->tm_year = rtcDate.Year - 1900;
  Serial.printf("\n   ctime Year: %i", ctime->tm_year);
  ctime->tm_mon = rtcDate.Month - 1;
  Serial.printf("\n   ctime Month: %i", ctime->tm_mon);
  ctime->tm_mday = rtcDate.Date;
  Serial.printf("\n   ctime Day: %i", ctime->tm_mday);
  ctime->tm_sec = rtcTime.Seconds;
  Serial.printf("\n   ctime Seconds: %i", ctime->tm_sec);
  ctime->tm_min = rtcTime.Minutes;
  Serial.printf("\n   ctime Minutes: %i", ctime->tm_min);
  ctime->tm_hour = rtcTime.Hours;
  Serial.printf("\n   ctime Hours: %i", ctime->tm_hour);
  Serial.print("\nexiting convertRTCToCTm");
}

void setRTCTime(tm *timeinfo) {
  Serial.println("setRTCTime");
  RTC_TimeTypeDef RTCtime;
  RTC_DateTypeDef RTCDate;
  RTCtime.Minutes = timeinfo->tm_min;
  RTCtime.Seconds = timeinfo->tm_sec;
  RTCtime.Hours = timeinfo->tm_hour;
  RTCDate.Year = timeinfo->tm_year + 1900;
  RTCDate.Month = timeinfo->tm_mon + 1;
  RTCDate.Date = timeinfo->tm_mday;
  RTCDate.WeekDay = timeinfo->tm_wday;

  M5.rtc.SetTime(&RTCtime);
  M5.rtc.SetDate(&RTCDate);
}

void formatRTC(RTC rtc, const char *dateFormatString, const char *timeFormatString, char *dateBuffer, char *timeBuffer, int dateBufferSize, int timeBufferSize) {
  Serial.print("\nformatRTC");
  struct tm ctime;
  convertRTCToCTm(rtc, &ctime);

  Serial.printf("\n   strftime -- DateBufferSize: %i, DateFormatString: %s", dateBufferSize, dateFormatString);
  strftime(dateBuffer, dateBufferSize, dateFormatString, &ctime);
  Serial.printf("\n   DateBuffer: %s", dateBuffer);
  Serial.printf("\n   strftime -- TimeBufferSize: %i, TimeFormatString: %s", timeBufferSize, timeFormatString);
  strftime(timeBuffer, timeBufferSize, timeFormatString, &ctime);
  Serial.printf("\n   TimeBuffer: %s", timeBuffer);

  Serial.print("\nexit formatRTC");
}

float getBatteryVoltage() {
  analogSetPinAttenuation(35, ADC_11db);
  esp_adc_cal_characteristics_t *adc_chars =
    (esp_adc_cal_characteristics_t *)calloc(
      1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 3600,
                           adc_chars);
  uint16_t adcValue = analogRead(35);

  return float(esp_adc_cal_raw_to_voltage(adcValue, adc_chars)) * 25.1 / 5.1 /
         1000;
}

bool initialize() {

  Wire1.begin(21, 22);
  uint8_t rtcData = M5.rtc.ReadReg(0x01);

  M5.begin();
  preferences.begin("bcmicro", false);
  digitalWrite(LED_EXT_PIN, LOW);

  if (!M5.M5Ink.isInit()) {
    Serial.printf("Ink Init faild");
    while (1)
      delay(100);
  }

  delay(1000);
  if (InkPageSprite.creatSprite(0, 0, 200, 200, true) != 0) {
    Serial.printf("Ink Sprite create faild");
  }
  return ((rtcData & 0b00000100) == 0b00000100);
}

void getPreferenceString(const char *key, char *buf, char *defaultValue, uint8_t len, bool *exists) {
  if (preferences.isKey(key)) {
    preferences.getString(key, defaultValue).toCharArray(buf, len);
    *exists = true;
  }
}

void getPreferenceLong64(const char *key, int64_t *value, int defaultValue, bool *exists) {
  if (preferences.isKey(key)) {
    *value = preferences.getLong64(key, defaultValue);
    *exists = true;
  }
}

AppState get_saved_app_state(Ink_Sprite *sprite) {
  Serial.println("get_saved_app_state");
  AppState state = DefaultAppState;

  bool preferences_exist = false;
  bool *preferences_exist_ptr = &preferences_exist;

  getPreferenceString("price", state.price, "", 8, preferences_exist_ptr);
  getPreferenceString("chainTip", state.chainTip, "", 10, preferences_exist_ptr);
  getPreferenceString("mempoolSize", state.mempoolSize, "", 5, preferences_exist_ptr);
  getPreferenceString("nextBlockFee", state.nextBlockFee, "", 5, preferences_exist_ptr);
  getPreferenceString("date", state.date, "", 10, preferences_exist_ptr);
  getPreferenceString("time", state.time, "", 10, preferences_exist_ptr);
  getPreferenceLong64("ntpMRU", &state.lastNTPUpdate, 0, preferences_exist_ptr);
  getPreferenceLong64("timeMRU", &state.lastTimeUpdate, 0, preferences_exist_ptr);
  getPreferenceLong64("metricsMRU", &state.lastMetricsUpdate, 0, preferences_exist_ptr);

  if (preferences_exist)
    state.source = PREFERENCES;

  return state;
}

void drawImageToSprite(int posX, int posY, image_t *imagePtr,
                       Ink_Sprite *sprite) {
  sprite->drawBuff(posX, posY, imagePtr->width, imagePtr->height,
                   imagePtr->ptr);
}

void drawNumeric(const char price[], int posX, int posY, image_t *font,
                 Ink_Sprite *sprite, int decimalPlaces) {
  int pos = posX, rpad = 0, digits = 0, charIndex = 0;

  while (price[charIndex] != '\0' && digits < decimalPlaces) {
    int priceCharIndex = (price[charIndex] - '0');
    image_t *fontCharacter;
    bool isPeriod = false;

    if (priceCharIndex >= 0 && priceCharIndex <= 9) {
      fontCharacter = &(font[priceCharIndex]);
      digits = digits + 1;
    } else {
      fontCharacter = &period_icon;
      isPeriod = true;
    }

    pos = rpad + pos + fontCharacter->leftPadding;
    drawImageToSprite(pos, isPeriod ? posY - 1 : posY, fontCharacter, sprite);
    pos = pos + fontCharacter->width;
    rpad = fontCharacter->rightPadding;
    charIndex = charIndex + 1;
  }
}

void drawDateTime(char *displayDate, char *displayTime) {
  Serial.printf("\ndrawDateTime: %s     %s", displayDate, displayTime);
  InkPageSprite.drawString(5, 5, displayDate);
  InkPageSprite.drawString(80, 5, displayTime);
}

void drawPrice(char* price) {
  drawNumeric(price, 32, 87, seven_segment_26x42, &InkPageSprite, 6);
}

void drawBlockHeight(char* blockHeight) {
  drawNumeric(blockHeight, 40, 33, seven_segment_20x32, &InkPageSprite, 7);
}

void drawMempoolSize(char* mempoolSize) {
  drawNumeric(mempoolSize, 3, 154, seven_segment_20x32, &InkPageSprite, 3);
}

void drawNextBlockFee(char* nextBlockFee) {
  drawNumeric(nextBlockFee, 105, 154, seven_segment_20x32, &InkPageSprite, 3);
}

void drawBattery(Ink_Sprite *sprite) {
  float batteryVoltage1 = getBatteryVoltage();
  char voltageDiagnostic[32];
  sprite->drawString(
    5, 5, "                                                         ");

  int bp = (batteryVoltage1 < 3.2) ? 0 : (batteryVoltage1 - 3.2) * 100;
  sprintf(voltageDiagnostic, "%.5g: %i", batteryVoltage1, bp);

  uint8_t batteryIconIndex =
    bp >= 71 ? 4 : bp >= 51 ? 3 : bp >= 31 ? 2 : bp >= 11 ? 1 : 0;

  drawImageToSprite(165, 4, &battery[batteryIconIndex], sprite);
}

void draw_status_bar() {
  drawBattery(&InkPageSprite);
}

void draw_static_images() {
  drawImageToSprite(7, 35, &chaintip_icon, &InkPageSprite);
  drawImageToSprite(7, 89, &dollar_sign_icon, &InkPageSprite);
  drawImageToSprite(88, 156, &megabytes_icon, &InkPageSprite);
  drawImageToSprite(184, 156, &sats_icon, &InkPageSprite);

  InkPageSprite.FillRect(6, 24, 188, 1, 0);
  InkPageSprite.FillRect(6, 77, 188, 1, 0);
  InkPageSprite.FillRect(6, 143, 188, 1, 0);
  InkPageSprite.FillRect(101, 151, 1, 38, 0);
}

void draw_app_state(AppState appState) {

  Serial.println("\ndraw_app_state");
  Serial.printf("\nLast Metrics Update: %ld", appState.lastMetricsUpdate);
  Serial.printf("\nLast Time Update: %ld", appState.lastTimeUpdate);
  if (appState.lastMetricsUpdate == appState.lastTimeUpdate) {
    drawPrice(appState.price);
    drawBlockHeight(appState.chainTip);
    drawMempoolSize(appState.mempoolSize);
    drawNextBlockFee(appState.nextBlockFee);
  }

  drawDateTime(appState.date, appState.time);

  InkPageSprite.pushSprite();
}

void retrievePrice(HTTPClient *http, char *priceBuf) {
  Serial.print("\nRetrieve price");
  http->begin("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=usd&include_last_updated_at=true");
  int httpCode = http->GET();
  Serial.printf("\nHTTP Response Code: %i", httpCode);
  if (httpCode > 0) { // httpCode will be negative on error.

    if (httpCode == HTTP_CODE_OK) { // file found at server.

      String response = http->getString(); //.toCharArray(Buf, 256);
      int usd_index = response.lastIndexOf("usd") + 5;
      int usd_index_end = response.indexOf(",", usd_index);

      String parsedString = response.substring(usd_index, usd_index_end);

      parsedString.toCharArray(priceBuf, 256);
    }
    else {
      Serial.println("Price retrieval failed. HTTP Status not OK");

    }
  }
  else {
    Serial.println("Price retrieval failed. HTTP Status is < 0");
  }
  http->end();
}

void retrieveBlockHeight(HTTPClient *http, char *chainTipBuf) {
  Serial.print("\nretrieveBlockHeight");
  http->begin("https://mempool.space/api/blocks/tip/height");

  int httpCode = http->GET();
  Serial.printf("\nHTTP Response Code: %i", httpCode);
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      http->getString().toCharArray(chainTipBuf, 8);
    }
    else {
      Serial.println("Block height retrieval failed. HTTP Status not OK");
    }
  }
  else {
    Serial.println("Block height retrieval failed. HTTP Status is < 0");
  }
  http->end();
}

void retrieveMempoolSize(HTTPClient *http, char *mempoolSize) {
  Serial.print("\nretrieveMempoolSize");
  http->begin("https://mempool.space/api/v1/fees/mempool-blocks");
  int httpCode = http->GET();
  Serial.printf("\nHTTP Response Code: %i", httpCode);
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      DynamicJsonDocument doc(2048);
      deserializeJson(doc, http->getStream());
      JsonArray arr = doc.as<JsonArray>();
      int mempoolTotal = 0;
      for (JsonObject mempoolBlock : arr) {
        int mempoolBlockSize = mempoolBlock["blockSize"];
        mempoolTotal = mempoolTotal + mempoolBlockSize;
      }
      sprintf(mempoolSize, "%.2f", (float)mempoolTotal / (float)1000000);
    }
  }
  http->end();
}

void retrieveNextBlockFee(HTTPClient *http, char *nextBlockFee) {
  Serial.print("\nretrieveNextBlockFee");
  http->begin("https://mempool.space/api/v1/fees/recommended");
  int httpCode = http->GET();
  Serial.printf("\nHTTP Response Code: %i", httpCode);
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      DynamicJsonDocument doc(128);
      deserializeJson(doc, http->getStream());
      int fastestFee = doc["fastestFee"];
      sprintf(nextBlockFee, "%03d", fastestFee);
    }
  }
  http->end();
}

void retrieveNTPTime() {
  Serial.print("\nretrieveNTPTime");
  configTime(3600 * UTC_OFFSET, 3600, "us.pool.ntp.org");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("\nFailed to obtain time");
    return;
  }
  setRTCTime(&timeinfo);
}

void retrieveRTCTime(RTC rtc, char* date, int dateBufferSize, char* time, int timeBufferSize) {
  Serial.print("\nretrieveTime");
  formatRTC(M5.rtc, DATE_FORMAT, TIME_FORMAT, date, time, dateBufferSize, timeBufferSize);
}

AppState retrieveMetrics(AppState previousAppState) {

  Serial.print("\nretrieveMetrics");
  Serial.printf("\n MRU - time: %s", String((long)previousAppState.lastTimeUpdate));
  Serial.printf("\n MRU - NTP: %s", String((long)previousAppState.lastNTPUpdate));
  Serial.printf("\n MRU - metrics: %s", String((long)previousAppState.lastMetricsUpdate));

  AppState retrievedAppState;
  HTTPClient http;

  struct tm ctimeStruct;
  convertRTCToCTm(M5.rtc, &ctimeStruct);
  int64_t epoch = (int64_t)mktime(&ctimeStruct);
  Serial.printf("\nepoch: %ld", epoch);
  Serial.println("\nafter mktime");

  bool shouldRetrieveMetrics = ((int64_t)epoch - previousAppState.lastMetricsUpdate) >= 600,
       shouldRetrieveNTPTime = ((int64_t)epoch - previousAppState.lastNTPUpdate) >= 3600,
       requiresWifi = shouldRetrieveMetrics && shouldRetrieveNTPTime;

  if (requiresWifi) {

    uint32_t connect_timeout = millis() + 10000;
    WiFi.begin(WIFI_CONFIGURATION.ssid, WIFI_CONFIGURATION.password);
    while ((WiFi.status() != WL_CONNECTED) && (millis() < connect_timeout)) {
      Serial.println("WiFi connecting...");
      delay(500);
    }
    Serial.println("WiFi Connected");

    if (WiFi.status() == WL_CONNECTED) {

      if (shouldRetrieveMetrics) {
        http.setReuse(true);
        retrievePrice(&http, retrievedAppState.price);
        preferences.putString("price", String(retrievedAppState.price));
        delay(200);

        retrieveBlockHeight(&http, retrievedAppState.chainTip);
        preferences.putString("chainTip", String(retrievedAppState.chainTip));
        delay(200);

        retrieveMempoolSize(&http, retrievedAppState.mempoolSize);
        preferences.putString("mempoolSize", String(retrievedAppState.mempoolSize));
        delay(200);

        retrieveNextBlockFee(&http, retrievedAppState.nextBlockFee);
        preferences.putString("nextBlockFee", String(retrievedAppState.nextBlockFee));

        http.end();
        retrievedAppState.lastMetricsUpdate = epoch;
        preferences.putLong64("metricsMRU", epoch);
      }

      if (shouldRetrieveNTPTime) {
        Serial.println("Retrieving NTP time");
        retrieveNTPTime();
        retrievedAppState.lastNTPUpdate = epoch;
        preferences.putLong64("ntpMRU", epoch);
      }
    }
    else {
      Serial.println("Wifi Connection failure!");
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }

  if (((int64_t)epoch - previousAppState.lastTimeUpdate) >= 60) {
    retrieveRTCTime(M5.rtc, retrievedAppState.date, 10, retrievedAppState.time, 10);
    retrievedAppState.lastTimeUpdate = epoch;
    preferences.putString("time", retrievedAppState.time);
    preferences.putString("date", retrievedAppState.date);
    preferences.putLong64("timeMRU", epoch);
  }
  return retrievedAppState;
}

void setup() {
  bool fromSleep = initialize(); // Powered on from RTC timer (as opposed to power button)
  AppState previousAppState = { .source = NEW_STATE };
  if (fromSleep) {
    previousAppState = get_saved_app_state(&InkPageSprite);
    draw_app_state(previousAppState);
  }
  else {
    preferences.clear();
    M5.M5Ink.clear();
    delay(1000);
    draw_static_images();
  }

  draw_status_bar();

  AppState retrievedAppState = retrieveMetrics(previousAppState);

  draw_app_state(retrievedAppState);
  InkPageSprite.pushSprite();
  M5.shutdown(60);
}

void loop() {
}
