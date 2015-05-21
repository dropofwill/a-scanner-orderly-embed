// RGB Sensor libraries
#include <Wire.h>
#include "Adafruit_TCS34725.h"
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X);

// NeoPixel Libraries
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
const int NEOPIXEL_COUNT = 16;
const int NEOPIXEL_PIN = 6;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NEOPIXEL_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);
uint32_t neo_red   = strip.Color(255, 0, 0);
uint32_t neo_green = strip.Color(0, 255, 0);
uint32_t neo_blue  = strip.Color(0, 0, 255);
uint32_t neo_off   = strip.Color(0,0,0);
uint32_t current_neocolor = neo_off;
uint16_t delay_counter = 0;
uint16_t color_counter = 0;
const int SLOW = 20;
const int MEDIUM = 10;
const int FAST = 2;

// Pressure sensor related variable and constants
const int FSR_PIN = A9;
const int FSR_CLICK_THRESHOLD = 15;
const int FSR_COUNT_THRESHOLD = 1000;
const int LOOPS_PER_LED = int(1000.0f/16.0f);
//const int LOOPS_PER_LED = int(float(FSR_CLICK_THRESHOLD) / float(NEOPIXEL_COUNT));
// For how many loops has the pressure sensor been reading above a certain threshold?
int fsr_counter = 0;
int fsr_val = 0;

/*
 * 0 = waiting to connect to node
 * 1 = waiting for drink order 1
 * 2 = waiting for drink order 2
 * 3 = sending drink order
 * 4 = waiting for drink started
 * 5 = waiting for drink finished
 * 6 = sending drink delivered
 * 7 = waiting to restart
 */
const int WAIT_CONNECT = 0;
const int WAIT_DRINK_1 = 1;
const int WAIT_DRINK_2 = 2;
const int SEND_DRINK_O = 3;
const int WAIT_DRINK_S = 4;
const int WAIT_DRINK_F = 5;
const int SEND_DRINK_D = 6;
const int WAIT_RESTART = 7;
int state = 0;

// 96 is '^', chosen because beginning of line in regex
const char OPEN  = 96;
// 36 is '$', chosen because end of line in regex
const char CLOSE = 36;

// What the server sends us to let us know a drink has been started and ended
const char BEGIN = 91; // '['
const char READY = 93; // ']'

/*
 * values 0-2 represent different drinks on the chart
 * -1 means not selected yet
 * Spirit
 *   0 => Vodka => Green
 *   1 => Gin   => Blue
 *   2 => Rum   => Red
 *
 * Mixer:
 *   0 => Lemon Lime Soda => Green
 *   1 => Orange          => Blue
 *   2 => Cranberry       => Red
 */
const int RED   = 0;
const int BLUE  = 1;
const int GREEN = 2;
//int spirit = -1;
//int mixer  = -1;
// Store as an array, spirit in 0, mixer in 1
int spirit_mixer[] = {-1, -1};
// Keep track of the current drink waiting to be selected
int current_drink_slot = 0;

boolean sending = false;

void setup() {
  Serial.begin(9600);
  tcs.begin();
  strip.begin();
}

void loop() {
  switch (state) {
    case WAIT_CONNECT:
      connect_to_server();
      break;
    case WAIT_DRINK_1:
    case WAIT_DRINK_2:
      process_drink_selection();
      break;
    case SEND_DRINK_O:
      send_drink_selection();
      break;
    case WAIT_DRINK_S:
      if (was_drink_started()) {
        state = WAIT_DRINK_F;
      }
      break;
    case WAIT_DRINK_F:
      if (was_drink_finished()) {
        state = SEND_DRINK_D;
      }
      break;
    case SEND_DRINK_D:
      if (was_drink_delivered()) {
        state = WAIT_RESTART;
      }
      else {
        Serial.println("delivered");
      }
      break;
    case WAIT_RESTART:
      clear_neopixels();
      state = WAIT_DRINK_1;
      break;
  }
}

// Handles state WAIT_CONNECT
// Send a connect message until the server responds
void connect_to_server() {
  if (message_was_received()) {
    state = WAIT_DRINK_1;
  }
  else {
    // Throw out a request to connect until the server responds with a '$'
    Serial.println("connect");
  }
}

// Handles state WAIT_DRINK_1 & WAIT_DRINK_2
void process_drink_selection() {
   if (was_clicked()) {
     spirit_mixer[current_drink_slot] = get_current_color();
     Serial.println(spirit_mixer[current_drink_slot]);
     current_drink_slot++;

     if (current_drink_slot > 1) {
       state = SEND_DRINK_O;
     }
   }
}

void send_drink_selection() {
  stateful_rainbow(SLOW);

  String json = "{\"drink\": [";
  json += spirit_mixer[0];
  json += ",";
  json += spirit_mixer[1];
  json += "]}";
  Serial.println(json);
  if (message_was_received()) {
    state = WAIT_DRINK_S;
  }
}

boolean was_clicked() {
  fsr_val = analogRead(FSR_PIN);
  Serial.println(fsr_val);

  // Light up this many leds
  // const int LOOPS_PER_LED = FSR_CLICK_THRESHOLD / NEOPIXEL_COUNT;
  if (fsr_val > FSR_CLICK_THRESHOLD) {
    fsr_counter++;
    int pins_to_light = int(fsr_counter / LOOPS_PER_LED);
//    Serial.println(pins_to_light);
    color_wipe(pins_to_light, current_neocolor);
  }
  else {
    fsr_counter = 0;
    current_neocolor = get_current_neocolor();
    clear_neopixels();
  }

  if (fsr_counter > FSR_COUNT_THRESHOLD) {
    fsr_counter = 0;
    clear_neopixels();
    return true;
  }
  else {
    return false;
  }
}

int get_current_color() {
  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  if (r > g && r > b) {
    return RED;
  }
  else if (g > b && g > r) {
    return GREEN;
  }
  else if (b > g && b > r) {
    return BLUE;
  }
}

uint32_t get_current_neocolor() {
  switch (get_current_color()) {
    case RED:
      return neo_red;
    case GREEN:
      return neo_green;
    case BLUE:
      return neo_blue;
  }
}

boolean was_drink_started() {
  stateful_rainbow(SLOW);
  return last_char_was(BEGIN);
}

boolean was_drink_finished() {
  stateful_rainbow(FAST);
  return last_char_was(READY);
}

boolean was_drink_delivered() {
  stateful_rainbow(SLOW);
  fsr_val = analogRead(FSR_PIN);
  /* Serial.println(fsr_val); */

  // Light up this many leds
  // const int LOOPS_PER_LED = FSR_CLICK_THRESHOLD / NEOPIXEL_COUNT;
  if (fsr_val > FSR_CLICK_THRESHOLD) {
    fsr_counter++;
  }
  else {
    fsr_counter = 0;
    clear_neopixels();
  }

  if (fsr_counter > FSR_COUNT_THRESHOLD) {
    fsr_counter = 0;
    clear_neopixels();
    return true;
  }
  else {
    return false;
  }
}

// Returns true if we get the close char ("$"), false otherwise
boolean message_was_received() {
  return last_char_was(CLOSE);
}

boolean last_char_was(char character) {
  char in_char = Serial.read();
  if (in_char == character) {
    return true;
  }
  else {
    return false;
  }
}

void stateful_rainbow(int delay_threshold) {
  uint16_t i, j;
  j = color_counter;

  for(i=0; i< strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
  }
  strip.show();

  if (delay_counter < delay_threshold) {
    delay_counter++;
  }
  else {
    delay_counter = 0;
    if (color_counter < 256) {
      color_counter++;
    }
    else {
      color_counter = 0;
    }
  }
}

void color_wipe(int number, uint32_t c) {
  for(uint16_t i = 0; i < number; i++) {
      strip.setPixelColor(i, c);
  }
  strip.show();
}

void clear_neopixels() {
  color_wipe(NEOPIXEL_COUNT, neo_off);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}
