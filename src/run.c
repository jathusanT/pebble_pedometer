/*
 * pebble pedometer +
 * @author jathusant
 */

#include <run.h>

Window *window;
ActionBarLayer *cadenza_player;

static SimpleMenuLayer pedometer_settings;
static SimpleMenuItem s_fruit_menu_items[4];
static SimpleMenuItem s_animal_menu_items[4];
static SimpleMenuSection s_menu_sections[2];
static char *s_fruit_names[4] = {"Start", "Set Limit", "Theme", "About"};
static char *s_animal_names[4] = {"Run", "Walk", "Jog", "Swim"};

TextLayer *main_message;
TextLayer *main_message2;
TextLayer *sub_message;
TextLayer *sub_message2;
bool isDark = true;
int pedometerCount = 0;

void up_click_handler_player(ClickRecognizerRef recognizer, void *context){
}
 
void down_click_handler_player(ClickRecognizerRef recognizer, void *context){
}
 
void select_click_handler_player(ClickRecognizerRef recognizer, void *context){
}

void click_config_provider_audio(void *context){
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler_player);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler_player);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler_player);
}

void handleColorSelect(){
    if (isDark){
        isDark = false;
        window_set_background_color(window, GColorWhite);
        text_layer_set_text_color(main_message, GColorBlack);
        text_layer_set_text_color(main_message2, GColorBlack);
        text_layer_set_text_color(sub_message, GColorBlack);
        text_layer_set_text_color(sub_message2, GColorBlack);
    } else {
        isDark = true;
        window_set_background_color(window, GColorBlack);
        text_layer_set_text_color(main_message, GColorWhite);
        text_layer_set_text_color(main_message2, GColorWhite);
        text_layer_set_text_color(sub_message, GColorWhite);
        text_layer_set_text_color(sub_message2, GColorWhite);
    }   
}

void up_click_handler(ClickRecognizerRef recognizer, void *context){
    handleColorSelect();
}
 
void down_click_handler(ClickRecognizerRef recognizer, void *context){
    handleColorSelect();
}
 
void select_click_handler(ClickRecognizerRef recognizer, void *context){
    vibes_double_pulse();
    action_bar_layer_add_to_window(cadenza_player, window);
    if (isDark){
        action_bar_layer_set_background_color(cadenza_player, GColorWhite);
    }
	text_layer_destroy(main_message);
    text_layer_destroy(main_message);
    text_layer_destroy(main_message2);
    text_layer_destroy(sub_message);
    text_layer_destroy(sub_message2);
    action_bar_layer_set_click_config_provider(cadenza_player, click_config_provider_audio);
}

void click_config_provider(void *context)
{
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

void window_load(Window *window){
    //Setting up Cadenza Player Layer
    //pedometerMenu = simple_menu_layer_create(GRect(0, 0, 150, 150), window, menu_sections, 4, void *callback_context);
    cadenza_player = action_bar_layer_create();
	
    window_set_background_color(window, GColorBlack);
    
    //initializing text layers
    main_message = text_layer_create(GRect(0, 0, 150, 150));
    main_message2 = text_layer_create(GRect(0, 30, 150, 150));
    sub_message = text_layer_create(GRect(0,60,150,150));
    sub_message2 = text_layer_create(GRect(0,110,150,150));
    
    //"Welcome"
    text_layer_set_background_color(main_message, GColorClear);
    text_layer_set_text_color(main_message, GColorWhite);
    text_layer_set_font(main_message, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_LT_30)));
    layer_add_child(window_get_root_layer(window), (Layer*) main_message);
    
    //"to Cadenza"
    text_layer_set_background_color(main_message2, GColorClear);
    text_layer_set_text_color(main_message2, GColorWhite);
    text_layer_set_font(main_message2, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_LT_15)));
    layer_add_child(window_get_root_layer(window), (Layer*) main_message2);
    
    //"select a theme"
    text_layer_set_background_color(sub_message, GColorClear);
    text_layer_set_text_color(sub_message, GColorWhite);
    text_layer_set_font(sub_message, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_LT_15)));
    layer_add_child(window_get_root_layer(window), (Layer*) sub_message);
    
    //"current theme:"
    text_layer_set_background_color(sub_message2, GColorClear);
    text_layer_set_text_color(sub_message2, GColorWhite);
    text_layer_set_font(sub_message2, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTO_LT_15)));
    layer_add_child(window_get_root_layer(window), (Layer*) sub_message2);
    
    //setting text
    text_layer_set_text(main_message, " Welcome");
    text_layer_set_text(main_message2, "      to Pedometer+");
    text_layer_set_text(sub_message, "        UP & DOWN\n    to change theme.");
    text_layer_set_text(sub_message2, "     SELECT to start\n            moving!");
} 

void window_unload(Window *window){
    text_layer_destroy(main_message);
    text_layer_destroy(main_message);
    text_layer_destroy(main_message2);
    text_layer_destroy(sub_message);
    text_layer_destroy(sub_message2);
}

void handle_init(void) {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
      .load = window_load,
      .unload = window_unload,
    });
    window_set_click_config_provider(window, click_config_provider);
    window_stack_push(window, true);
}

void handle_deinit(void) {
    window_destroy(window);
}

int main(void) {
    handle_init();
    app_event_loop();
    handle_deinit();
}