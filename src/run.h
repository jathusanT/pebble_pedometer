/*
 * pebble pedometer
 * run header file
 * @author jathusant
 */

#ifndef RUN_H
#define RUN_H
	
#include <pebble.h>

void setup_menu_items();
void setup_menu_sections();
void setup_menu_window();
void settings_load();
void settings_unload();
void setup_menu_items();
void setup_menu_sections();
void m_up_click_handler(ClickRecognizerRef recognizer, void *context);
void m_down_click_handler(ClickRecognizerRef recognizer, void *context);
void m_select_click_handler(ClickRecognizerRef recognizer, void *context);
void menu_click_config_provider(void *context);
	
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