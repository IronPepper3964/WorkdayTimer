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


#define TOP_BUTTON_PIN  35 //Top/Right button
#define BOTTOM_BUTTON_PIN 0 //Bottom/Left button

#define BACKLIGHT_PIN tft.TFT_BL //Pin that controls the backlight

Button2 top_button;
Button2 bottom_button;

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library


/******variable definitions*****/
float percent_complete;
int total_time_hours = 7;
int total_time_minutes = 30;
int total_time_seconds = 0;
int workday_ms_total = ((((total_time_hours*60)+total_time_minutes)*60)+total_time_seconds)*1000;
int start_ms;

long total_working_ms = 0;
long current_working_ms_start = 0;
long total_break_ms = 0;
long current_break_ms_start = 0;
long total_paid_break_bank_ms = (30*60*1000); //30 minutes paid break per 8 hours
long current_times_ms_start = 0;

int display_update_interval_ms = (1000/30);
long last_diplay_update_ms = 0;

long last_full_diplay_update_ms = 0;
long full_dispaly_update_interval_ms = 10000;


boolean default_display = true;
boolean currently_working = false;
boolean currently_on_break = false;

String program_version = "0.0.1";
String default_display_banner_text = "Work Timer - v" + default_display_banner_text;

void debug_display_text(String in_str, int in_x=0, int in_y=80);
String ms_to_time(long in_ms, boolean include_ms=false);

/*******Setup******/
void setup() {

    // Screen setup....
    tft.init();
    tft.setRotation(1); // Rotate the screen to landscape

    // Backlight Setup
    pinMode(TFT_BL, OUTPUT);
    ledcSetup(0, 5000, 8); // 0-15, 5000, 8
    ledcAttachPin(TFT_BL, 0); // TFT_BL, 0 - 15
    adjust_backlight(255);

    tft.fillScreen(TFT_BLACK);

    // Set "cursor" at top left corner of display (0,0) and select font 4
  tft.setCursor(0, 0, 4);
  tft.setTextSize(1);

  // Set the font colour to be white with a black background
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  // Print some text
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.print(default_display_banner_text);
  //tft.setTextColor(TFT_RED, TFT_BLACK);
  //tft.println("Work Timer");
    tft.setTextSize(1);

  Serial.begin(115200);
  delay(50);

  Serial.print("\n\nWorkday Timer \nVersion: ");
  Serial.print(default_display_banner_text);

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
    if(full_dispaly_update_interval_ms>(millis()-last_full_diplay_update_ms)){
        update_default_display(true);
        last_full_diplay_update_ms = millis();
    }else
        update_default_display(false);
    last_diplay_update_ms = millis();
  }

  /*}else{
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

    adjust_backlight(40);

    if(currently_on_break){
        end_timing_break();
    }
    if(!currently_working){
        start_timing_work();
    }

    //Update the full screen...
    update_default_display(true);
}
void btap(Button2& btn) {
    Serial.println("btap");
    Serial.println(btn.getAttachPin());
    debug_display_text("btap");
    adjust_backlight(255);

    if(currently_working){
        end_timing_work();
    }
    if(!currently_on_break){
        start_timing_break();
    }

    //Update the full screen...
    update_default_display(true);

    
}

void debug_display_text(String in_str, int in_x, int in_y){
    tft.setCursor(in_x, in_y, 4);
    tft.setTextSize(1);
    tft.println(in_str);
}

void start_timing_work(){
    if(!currently_working){
        current_working_ms_start = millis();
        tft.setTextColor(TFT_RED, TFT_BLACK);
        default_display_banner_text = "Working      ";
        currently_working = true;
    }
}

void end_timing_work(){
    if(currently_working){
        total_working_ms = total_working_ms + (millis() - current_working_ms_start);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        default_display_banner_text = "    Work End      ";
        currently_working = false;
    }
}

void start_timing_break(){
    if(!currently_on_break){
        current_break_ms_start = millis();
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        default_display_banner_text = "On Break...";
        currently_on_break = true;
    }
}

void end_timing_break(){
    if(currently_on_break){
        total_break_ms = total_break_ms + (millis() - current_break_ms_start);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        default_display_banner_text = "    Break End      ";
        currently_on_break = false;
    }
}

////////////
// Updates the display that shows how much time is remaining
///////////
void update_default_display(boolean update_full_screen){
    if(update_full_screen){
        if(workday_ms_total<=get_total_working_ms()){
            tft.fillScreen(TFT_WHITE);
            adjust_backlight(255);
            display_update_interval_ms = 1000;
        }
    
    //Display the banner...
    tft.setCursor(0, 0, 4);
    if(currently_working)
        tft.setTextColor(TFT_BLUE, TFT_BLACK);
    else
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.print(default_display_banner_text);
    }

    //Display Total working time...
    if(update_full_screen || currently_working){
        tft.setCursor(0, 50, 4);
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.print(ms_to_time(get_total_working_ms(),false));

        tft.print(" | ");
        tft.print(ms_to_time(workday_ms_total-get_total_working_ms()));
    }
    

    //Display Total break time...
    if(update_full_screen || currently_on_break){
        tft.setCursor(0, 80, 4);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.print(ms_to_time(get_total_break_ms(),false));
    }

    //debug_display_text(ms_to_time(get_total_break_ms()));
}

void adjust_backlight(int intensity){
    ledcWrite(0, intensity); // 0-15, 0-255 (with 8 bit resolution); 0=totally dark;255=totally shiny
}

////////////
// Returns the current total ms breaks taken - taking into account current working ms totals
///////////
long get_total_break_ms(){
    if(currently_on_break){
        return (total_break_ms + (millis() - current_break_ms_start));
    }else{
        return total_break_ms;
    }
}

////////////
// Returns the current total ms worked - taking into account current working ms totals
///////////
long get_total_working_ms(){
    if(currently_working){
        return total_working_ms + (millis() - current_working_ms_start);
    }else{
        return total_working_ms;
    }
}

////////////
// Changes milliseconds into time in hh:mm:ss:mmmm
// include_ms changins if milliseconds are added or not.
///////////
String ms_to_time(long in_ms, boolean include_ms){
    String return_str = "";
    String preceding_string = "";

    if(in_ms < 0){
        preceding_string = "-";
        in_ms = abs(in_ms);
    }
    //long tmp_seconds_remaining = in_ms/1000;
    long tmp_ms_remaining = in_ms;///1000;
    int hours = tmp_ms_remaining/3600000;

    tmp_ms_remaining = tmp_ms_remaining%3600000;

    int minutes = tmp_ms_remaining/60000;
    tmp_ms_remaining = tmp_ms_remaining%60000;

    int seconds = tmp_ms_remaining/1000;
    int ms = tmp_ms_remaining%1000;

    char buffer [50];
    if(include_ms){
        sprintf (buffer, "%s%01d:%02d:%02d:%03d", preceding_string, hours, minutes, seconds, ms);
    }else{
        sprintf (buffer, "%s%01d:%02d:%02d", preceding_string, hours, minutes, seconds);
    }

    return_str = String(buffer);
    return return_str;
}


  