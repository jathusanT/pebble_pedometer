/*
 * pebble pedometer +
 * run header file
 * @author jathusant
 */

#ifndef RUN_H
#define RUN_H
	
#include <pebble.h>

void up_click_handler_player(ClickRecognizerRef recognizer, void *context);
void down_click_handler_player(ClickRecognizerRef recognizer, void *context);
void select_click_handler_player(ClickRecognizerRef recognizer, void *context);
void click_config_provider_audio(void *context);
void handleColorSelect();
void up_click_handler(ClickRecognizerRef recognizer, void *context);
void down_click_handler(ClickRecognizerRef recognizer, void *context);
void select_click_handler(ClickRecognizerRef recognizer, void *context);
void click_config_provider(void *context);
void window_load(Window *window);
void window_unload(Window *window);
void handle_init(void);

void handle_deinit(void);
	
#endif