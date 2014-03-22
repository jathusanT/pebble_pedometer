/*
 * pebble pedometer
 * run header file
 * @author jathusant
 */

#ifndef RUN_H
#define RUN_H

#include <pebble.h>

char* determineCal(int cal);
void window_load(Window *window);
void window_unload(Window *window);
void pedometer_update();
void resetUpdate();
void update_ui_callback();
static void timer_callback(void *data);
void ped_load(Window *window);
void ped_unload(Window *window);
void start_callback(int index, void *ctx);
void info_load(Window *window);
void info_unload(Window *window);
void info_callback(int index, void *ctx);
void inc_click_handler(ClickRecognizerRef recognizer, void *context);
void dec_click_handler(ClickRecognizerRef recognizer, void *context);
void set_click_handler(ClickRecognizerRef recognizer, void *context);
void goal_set_click_config(void *context);
void stepGoal_load(Window *window);
void stepGoal_unload(Window *window);
void stepGoal_callback(int index, void *ctx);
void theme_callback(int index, void *ctx);
void setup_menu_items();
void setup_menu_sections();
void setup_menu_window();
void settings_load(Window *window);
void settings_unload(Window *window);
void handle_init(void);
void handle_deinit(void);

#endif