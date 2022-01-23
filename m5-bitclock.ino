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
} DefaultAppState = {NEW_STATE, "", "", "", ""};

typedef struct AppStateStruct AppState;

Ink_Sprite InkPageSprite(&M5.M5Ink);
Preferences preferences;

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

void drawDateTime() {
  RTC_TimeTypeDef RTCtime;
  M5.rtc.GetTime(&RTCtime);

  RTC_DateTypeDef RTCdate;
  M5.rtc.GetDate(&RTCdate);

  char ch_arr[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
                       };

  char displayDate[7]; // 01 Mar 01:01
  char displayTime[6];

  sprintf(displayDate, "%02i %s", RTCdate.Date, ch_arr[RTCdate.Month - 1]);
  sprintf(displayTime, "%02i:%02i", RTCtime.Hours, RTCtime.Minutes);

  InkPageSprite.drawString(5, 5, displayDate);
  InkPageSprite.drawString(80, 5, displayTime);
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

void displayBattery(Ink_Sprite *sprite) {
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

void getPreferenceInt(const char *key, int *value, int defaultValue, uint8_t len, bool *exists) {
  if (preferences.isKey(key)) {
    *value = preferences.getInt(key, defaultValue);
    *exists = true;
  }
}

void retrievePrice(HTTPClient *http, char *priceBuf) {

  http->begin(
    "https://api.coingecko.com/api/v3/simple/"
    "price?ids=bitcoin&vs_currencies=usd&include_last_updated_at=true");
  int httpCode = http->GET();
  if (httpCode > 0) { // httpCode will be negative on error.

    if (httpCode == HTTP_CODE_OK) { // file found at server.

      String response = http->getString(); //.toCharArray(Buf, 256);
      int usd_index = response.lastIndexOf("usd") + 5;
      int usd_index_end = response.indexOf(",", usd_index);

      String parsedString = response.substring(usd_index, usd_index_end);

      parsedString.toCharArray(priceBuf, 256);
    }
  }
  http->end();
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

void retrieveBlockHeight(HTTPClient *http, char *chainTipBuf) {
  http->begin("https://mempool.space/api/blocks/tip/height");
  int httpCode = http->GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      http->getString().toCharArray(chainTipBuf, 8);
    }
  }
  http->end();
}

void retrieveMempoolSize(HTTPClient *http, char *mempoolSize) {
  http->begin("https://mempool.space/api/v1/fees/mempool-blocks");
  int httpCode = http->GET();
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
  http->begin("https://mempool.space/api/v1/fees/recommended");
  int httpCode = http->GET();
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

void setRTCTime(tm *timeinfo) {
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

void retrieveNTPTime() {
  configTime(3600 * UTC_OFFSET, 3600, "us.pool.ntp.org");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  setRTCTime(&timeinfo);
}

AppState retrieveMetrics() {

  AppState retrievedAppState;
  HTTPClient http;
  uint8_t wifi_run_status;

  uint32_t connect_timeout = millis() + 10000;
  WiFi.begin(WIFI_CONFIGURATION.ssid, WIFI_CONFIGURATION.password);
  while ((WiFi.status() != WL_CONNECTED) && (millis() < connect_timeout)) {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {

    http.setReuse(true);

    retrievePrice(&http, retrievedAppState.price);
    preferences.putString("lastPrice", String(retrievedAppState.price));
    delay(200);

    retrieveBlockHeight(&http, retrievedAppState.chainTip);
    preferences.putString("lastChainTip", String(retrievedAppState.chainTip));
    delay(200);

    retrieveMempoolSize(&http, retrievedAppState.mempoolSize);
    preferences.putString("lastMempoolSize", String(retrievedAppState.mempoolSize));
    delay(200);

    retrieveNextBlockFee(&http, retrievedAppState.nextBlockFee);
    preferences.putString("lastNextBlockFee", String(retrievedAppState.nextBlockFee));

    retrieveNTPTime();
    drawDateTime();

    http.end();
  }

  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  return retrievedAppState;
}

AppState get_saved_app_state(Ink_Sprite *sprite) {
  AppState state = DefaultAppState;

  bool preferences_exist = false;
  bool *preferences_exist_ptr = &preferences_exist;

  getPreferenceString("lastPrice", state.price, "", 8, preferences_exist_ptr);
  getPreferenceString("lastChainTip", state.chainTip, "", 10, preferences_exist_ptr);
  getPreferenceString("lastMempoolSize", state.mempoolSize, "", 5, preferences_exist_ptr);
  getPreferenceString("lastNextBlockFee", state.nextBlockFee, "", 5, preferences_exist_ptr);
  // getPreferenceString("lastDate", state.date, "", 7, preferences_exist_ptr);

  if (preferences_exist)
    state.source = PREFERENCES;

  return state;
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
  drawPrice(appState.price);
  drawBlockHeight(appState.chainTip);
  drawMempoolSize(appState.mempoolSize);
  drawNextBlockFee(appState.nextBlockFee);
  InkPageSprite.pushSprite();
}

void draw_status_bar() {
  displayBattery(&InkPageSprite);
}

void setup() {
  bool fromSleep = initialize(); // Powered on from RTC timer (as opposed to power button)
  if (fromSleep) {
    AppState previousAppState = get_saved_app_state(&InkPageSprite);
    draw_app_state(previousAppState);
  }
  else {
    preferences.clear();
    M5.M5Ink.clear();
    delay(1000);
    draw_static_images();
  }

  draw_status_bar();

  AppState retrievedAppState = retrieveMetrics();
  draw_app_state(retrievedAppState);
  InkPageSprite.pushSprite();

  M5.shutdown(600);
}

void loop() {
}
