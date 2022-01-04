/******************
 * Workday Timer
 * 
 * Caleb Kitchen
 * 
 * 1/3/2022
 * 
 * 
 * This code is meant to run on a ESP32 with a display
 * It is a timer that starts and stops with a button push, with notifications around workday schedules
 *
 * The code was written for a TTGO ESP32 board with display
 * ****************/

/******Includes****/

#include "Button2.h"
#include <SPI.h>
#include <TFT_eSPI.h>       // Hardware-specific library

/******Definitions*****/
#define PROGRAM_VERSION "0.0.1"

#define TOP_BUTTON_PIN  35 //Top/Right button
#define BOTTOM_BUTTON_PIN 0 //Bottom/Left button

#define BACKLIGHT_PIN tft.TFT_BL //Pin that controls the backlight

Button2 top_button;
Button2 bottom_button;

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library


/******variable definitions*****/
float percent_complete;
int total_time_hours = 8;
int total_time_minutes = 30;
int total_time_seconds = 0;
int total_timed_ms = ((((total_time_hours*60)+total_time_minutes)*60)+total_time_seconds)*1000;
int start_ms;

long total_working_ms = 0;
long total_paid_break_bank_ms = (30*60*1000); //30 minutes paid break per 8 hours
long current_times_ms_start = 0;

int display_update_interval_ms = 500;
long last_diplay_update_ms = 0;

boolean default_display = true;
boolean currently_working = false;
boolean currently_on_break = false;

void debug_display_text(String in_str, int in_x=0, int in_y=80);

/*******Setup******/
void setup() {

    // Screen setup....
    tft.init();
    tft.setRotation(1); // Rotate the screen to landscape

    tft.fillScreen(TFT_BLACK);

    // Set "cursor" at top left corner of display (0,0) and select font 4
  tft.setCursor(0, 0, 4);
  tft.setTextSize(1);

  // Set the font colour to be white with a black background
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Print some text
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print("Runtime counter\n");
  tft.setTextColor(TFT_RED, TFT_BLACK);
  tft.println("Work Timer");
    tft.setTextSize(1);

  Serial.begin(115200);
  delay(50);

  Serial.print("\n\nWorkday Timer \nVersion: ");
  Serial.print(PROGRAM_VERSION);

  top_button.begin(TOP_BUTTON_PIN);
  bottom_button.begin(BOTTOM_BUTTON_PIN);
  // button.setLongClickTime(1000);
  // button.setDoubleClickTime(400);

  Serial.println(" Longpress Time: " + String(top_button.getLongClickTime()) + "ms");
  Serial.println(" DoubleClick Time: " + String(bottom_button.getDoubleClickTime()) + "ms");

  top_button.setTapHandler(ttap);
  bottom_button.setTapHandler(btap);

  /*****Buttom Examples***
   * button.setChangedHandler(changed);
  button.setPressedHandler(pressed);
  button.setReleasedHandler(released);

   button.setTapHandler(tap);
  button.setClickHandler(click);
  button.setLongClickDetectedHandler(longClickDetected);
  button.setLongClickHandler(longClick);
  
  button.setDoubleClickHandler(doubleClick);
  button.setTripleClickHandler(tripleClick);
  *****/

 last_diplay_update_ms = millis();
}

/********Loop**********/
void loop() {
  top_button.loop();
  bottom_button.loop();

  //Passivly update screen every so often
  if (millis() >= (last_diplay_update_ms+display_update_interval_ms)){
    last_diplay_update_ms = millis();
    debug_display_text(String(millis()),0,100);
    debug_display_text(String((last_diplay_update_ms+display_update_interval_ms)),0,60);
  }/*else{
    debug_display_text("---"+String(millis()),0,100);
    debug_display_text(String((last_diplay_update_ms+display_update_interval_ms)),0,60);
  }*/
}


/********Callback Functions*****/
void tap(Button2& btn) {
    Serial.println("tap");
    Serial.println(btn.getAttachPin());
    debug_display_text("tap");
}
void ttap(Button2& btn) {
    Serial.println("ttap");
    Serial.println(btn.getAttachPin());
    debug_display_text("ttap");

    pinMode(TFT_BL, OUTPUT);
ledcSetup(0, 5000, 8); // 0-15, 5000, 8
ledcAttachPin(TFT_BL, 0); // TFT_BL, 0 - 15
ledcWrite(0, 255); // 0-15, 0-255 (with 8 bit resolution); 0=totally dark;255=totally shiny
}
void btap(Button2& btn) {
    Serial.println("btap");
    Serial.println(btn.getAttachPin());
    debug_display_text("btap");

    pinMode(TFT_BL, OUTPUT);
ledcSetup(0, 5000, 8); // 0-15, 5000, 8
ledcAttachPin(TFT_BL, 0); // TFT_BL, 0 - 15
ledcWrite(0, 1); // 0-15, 0-255 (with 8 bit resolution); 0=totally dark;255=totally shiny
}

void debug_display_text(String in_str, int in_x, int in_y){
    tft.setCursor(in_x, in_y, 4);
    tft.setTextSize(1);
    tft.println(in_str);
}


  