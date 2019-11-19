//----------------------------------------------------------------------------------------------
// T174 Candle Simulator
// 32 LED version
// Tom Höglund 2018
// 32 NeoPixel versio
//-----------------------------------------------------------------------------------------------
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif
// pin definition
#define STRIP_PIN 2
#define LDR       A3


#define TOTAL_NBR_LED  16
#define NBR_PATTERNS    8
#define NBR_COLORS     16
#define VALUES_PER_PATTERN 64

#define RGB_RED   0
#define RGB_GREEN 1
#define RGB_BLUE  2


struct led_ctrl_struct{
   byte pattern_indx;
   byte pattern_value_indx;
   unsigned int interval;
   unsigned long last_millis;
   byte rounds;
   byte color_indx;
   byte color[3];
};

led_ctrl_struct led_ctrl[TOTAL_NBR_LED];

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(TOTAL_NBR_LED, STRIP_PIN, NEO_GRB + NEO_KHZ800);

byte norm_distr[NBR_PATTERNS][VALUES_PER_PATTERN] = {
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3, 7, 17, 35, 64, 105, 155, 204, 241, 255, 241, 204, 155, 105, 64, 35, 17, 7, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 3, 6, 11, 20, 35, 55, 83, 117, 155, 192, 225, 247, 255, 247, 225, 192, 155, 117, 83, 55, 35, 20, 11, 6, 3, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 5, 9, 14, 23, 35, 50, 71, 96, 124, 155, 185, 213, 235, 250, 255, 250, 235, 213, 185, 155, 124, 96, 71, 50, 35, 23, 14, 9, 5, 3, 2, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3, 5, 7, 11, 17, 24, 35, 47, 64, 83, 105, 129, 155, 180, 204, 225, 241, 251, 255, 251, 241, 225, 204, 180, 155, 129, 105, 83, 64, 47, 35, 24, 17, 11, 7, 5, 3, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
       {0, 0, 0, 0, 1, 1, 1, 2, 3, 4, 6, 8, 11, 15, 20, 27, 35, 44, 55, 68, 83, 99, 117, 135, 155, 174, 192, 210, 225, 238, 247, 253, 255, 253, 247, 238, 225, 210, 192, 174, 155, 135, 117, 99, 83, 68, 55, 44, 35, 27, 20, 15, 11, 8, 6, 4, 3, 2, 1, 1, 1, 0, 0, 0},
       {2, 2, 3, 4, 5, 7, 9, 11, 14, 18, 23, 28, 35, 42, 50, 60, 71, 83, 96, 110, 124, 139, 155, 170, 185, 200, 213, 225, 235, 244, 250, 254, 255, 254, 250, 244, 235, 225, 213, 200, 185, 170, 155, 139, 124, 110, 96, 83, 71, 60, 50, 42, 35, 28, 23, 18, 14, 11, 9, 7, 5, 4, 3, 2},
       {35, 39, 44, 49, 55, 61, 68, 75, 83, 91, 99, 108, 117, 126, 135, 145, 155, 164, 174, 183, 192, 201, 210, 218, 225, 232, 238, 243, 247, 251, 253, 255, 255, 255, 253, 251, 247, 243, 238, 232, 225, 218, 210, 201, 192, 183, 174, 164, 155, 145, 135, 126, 117, 108, 99, 91, 83, 75, 68, 61, 55, 49, 44, 39},
       {105, 111, 117, 123, 129, 135, 142, 148, 155, 161, 168, 174, 180, 186, 192, 198, 204, 210, 215, 220, 225, 230, 234, 238, 241, 244, 247, 250, 251, 253, 254, 255, 255, 255, 254, 253, 251, 250, 247, 244, 241, 238, 234, 230, 225, 220, 215, 210, 204, 198, 192, 186, 180, 174, 168, 161, 155, 148, 142, 135, 129, 123, 117, 111} 
   }; 

byte col_rgb[NBR_COLORS][3] =
    { {255, 50, 10},  // 0
      {255,100, 20},
      {255,190,100},
      {255,100, 20},
      {255,150, 80},
      {255,190,100},
      {255,150, 50},
      {255,  0,  0},   // 7
      {  0,  0,255},   // 8
      {  0,255,  0},
      {  0,  0,255},
      {255,190,100},
      {255,190, 90}, 
      {255,190, 90},
      {255,190, 90},
      {255,255,255}    //15
    };
  
// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

//-----------------------------------------------------------------------------------
void setup() {
  delay(2000);  
  Serial.begin(9600);
  Serial.println("T174 32 LED candle v2 2018");
  
  for (byte i= 0;i < TOTAL_NBR_LED;i++) {
     update_led(i);
     for (uint8_t color = RGB_RED; color <= RGB_BLUE; color++){
       led_ctrl[i].color[color] = 0;
    } 
  }
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  randomSeed(analogRead(0));
}
//-----------------------------------------------------------------------------------
void loop() {
  for( byte i = 0; i < TOTAL_NBR_LED; i++){
    if (millis()-led_ctrl[i].last_millis > led_ctrl[i].interval){
       led_ctrl[i].last_millis = millis();
       if (++led_ctrl[i].pattern_value_indx >= VALUES_PER_PATTERN){                
          if (--led_ctrl[i].rounds == 0 ) {
             update_led(i);
          }
          else led_ctrl[i].pattern_value_indx=0; 
       }
   
       for (uint8_t color_indx_rgb = RGB_RED; color_indx_rgb <= RGB_BLUE; color_indx_rgb++){    
          led_ctrl[i].color[color_indx_rgb] = (byte)((uint16_t)col_rgb[led_ctrl[i].color_indx][color_indx_rgb] * 
                            (uint16_t) norm_distr[led_ctrl[i].pattern_indx][led_ctrl[i].pattern_value_indx] >> 8);
       } 
       strip.setPixelColor(i,strip.Color(led_ctrl[i].color[RGB_RED],led_ctrl[i].color[RGB_GREEN],led_ctrl[i].color[RGB_BLUE]));
    }
  }
  strip.show(); 
 
}

void update_led(byte led_indx){
   
   led_ctrl[led_indx].pattern_indx = random(2,NBR_PATTERNS);
   led_ctrl[led_indx].pattern_value_indx = 0;   //   random(0,VALUES_PER_PATTERN-1);
   led_ctrl[led_indx].color_indx = random(0,32);
   if (led_ctrl[led_indx].color_indx >= NBR_COLORS) led_ctrl[led_indx].color_indx = NBR_COLORS-1;   //dominating color
   led_ctrl[led_indx].interval = random(16,160);
   led_ctrl[led_indx].rounds = random(1,5);
   led_ctrl[led_indx].last_millis = millis();
}
