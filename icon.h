#include "utility/Ink_Sprite.h"
typedef struct {
  int width;
  int height;
  int bitCount;
  unsigned char *ptr;
  int leftPadding;
  int rightPadding;
} image_t;

extern unsigned char seven_segment_26x42_0[192]; //"0"
extern unsigned char seven_segment_26x42_1[192]; //"1"
extern unsigned char seven_segment_26x42_2[192]; //"2"
extern unsigned char seven_segment_26x42_3[192]; //"3"
extern unsigned char seven_segment_26x42_4[192]; //"4"
extern unsigned char seven_segment_26x42_5[192]; //"5"
extern unsigned char seven_segment_26x42_6[192]; //"6"
extern unsigned char seven_segment_26x42_7[192]; //"7"
extern unsigned char seven_segment_26x42_8[192]; //"8"
extern unsigned char seven_segment_26x42_9[192]; //"9"

extern unsigned char seven_segment_20x32_0[108]; //"0"
extern unsigned char seven_segment_20x32_1[108]; //"1"
extern unsigned char seven_segment_20x32_2[108]; //"2"
extern unsigned char seven_segment_20x32_3[108]; //"3"
extern unsigned char seven_segment_20x32_4[108]; //"4"
extern unsigned char seven_segment_20x32_5[108]; //"5"
extern unsigned char seven_segment_20x32_6[108]; //"6"
extern unsigned char seven_segment_20x32_7[108]; //"7"
extern unsigned char seven_segment_20x32_8[108]; //"8"
extern unsigned char seven_segment_20x32_9[108]; //"9"

extern image_t seven_segment_26x42[10];
extern image_t seven_segment_20x32[10];

extern unsigned char battery_0[56];   //"0"
extern unsigned char battery_25[56];  //"25"
extern unsigned char battery_50[56];  //"50"
extern unsigned char battery_75[56];  //"75"
extern unsigned char battery_100[56]; //"100"

extern image_t battery[5];

// extern image_t battery_small_100_icon;
extern image_t chaintip_icon;
extern image_t dollar_sign_icon;
extern image_t period_icon;
extern image_t megabytes_icon;
extern image_t sats_icon;
