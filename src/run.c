/*
 * pebble pedometer
 * @author jathusant
 */

#include <pebble.h>
#include <run.h>
#include <sys/time.h>
#include <math.h>

//interval for each accelerometer check
#define ACCEL_STEP_MS 450
	
Window *window;
Window *menu_window;
Window *set_stepGoal;
Window *pedometer;
Window *dev_info;

SimpleMenuLayer *pedometer_settings;
SimpleMenuItem menu_items[6];
SimpleMenuSection menu_sections[1];

char *item_names[6] = 
	{ "Start", "Step Goal", "Overall Steps", "Theme", "Version", "About" };
char *item_sub[6] = 
	{ "Lets Exercise!", "Not Set", "0 Total", "Current: Error Loading", "v1.0-RELEASE", "(c) Jathusan T" };

ActionBarLayer *stepGoalSetter;

static AppTimer *timer;

TextLayer *main_message;
TextLayer *main_message2;
TextLayer *hitSel;
TextLayer *stepGoalVisualizer;
TextLayer *steps;
TextLayer *pedCount;
TextLayer *infor;

static GBitmap *btn_dwn;
static GBitmap *btn_up;
static GBitmap *btn_sel;
static GBitmap *statusBar;
GBitmap *pedometerBack;
BitmapLayer *pedometerBack_layer;
GBitmap *splash;
BitmapLayer *splash_layer;

const int STEPS_PER_CALORIE = 20;
const int X_DELTA = 200;
const int Y_DELTA = 140;
const int Z_DELTA = 65;

char *theme;
bool saveIsDark;
bool isDark = true;
bool startedSession = false;
int stepGoal = 0;
int pedometerCount = 0;
int totalSteps;
int totalSteps_SAVE;
const int STEP_INCREMENT = 100;
int lastX, lastY, lastZ = 0;
int currX, currY, currZ = 0;
bool validX, validY, validZ = false;

/* code */

void window_load(Window *window) {

	splash = gbitmap_create_with_resource(RESOURCE_ID_SPLASH);
	window_set_background_color(window, GColorBlack);

	splash_layer = bitmap_layer_create(GRect(0, 0, 145, 185));
	bitmap_layer_set_bitmap(splash_layer, splash);
	layer_add_child(window_get_root_layer(window),
			bitmap_layer_get_layer(splash_layer));

	main_message = text_layer_create(GRect(0, 0, 150, 170));
	main_message2 = text_layer_create(GRect(3, 35, 150, 170));
	hitSel = text_layer_create(GRect(3, 40, 200, 170));

	text_layer_set_background_color(main_message, GColorClear);
	text_layer_set_text_color(main_message, GColorWhite);
	text_layer_set_font(main_message,
			fonts_load_custom_font(
					resource_get_handle(RESOURCE_ID_ROBOTO_LT_30)));
	layer_add_child(window_get_root_layer(window), (Layer*) main_message);

	text_layer_set_background_color(main_message2, GColorClear);
	text_layer_set_text_color(main_message2, GColorWhite);
	text_layer_set_font(main_message2,
			fonts_load_custom_font(
					resource_get_handle(RESOURCE_ID_ROBOTO_LT_15)));
	layer_add_child(window_get_root_layer(window), (Layer*) main_message2);

	text_layer_set_background_color(hitSel, GColorClear);
	text_layer_set_text_color(hitSel, GColorWhite);
	text_layer_set_font(hitSel,
			fonts_load_custom_font(
					resource_get_handle(RESOURCE_ID_ROBOTO_LT_15)));
	layer_add_child(window_get_root_layer(window), (Layer*) hitSel);

	
	text_layer_set_text(main_message, "     GOAL");
	text_layer_set_text(main_message2, "          Reached!");
	text_layer_set_text(hitSel, "\n\n\n\n\n\n        Press Back...");
}

void window_unload(Window *window) {
	text_layer_destroy(main_message);
	text_layer_destroy(main_message2);
	text_layer_destroy(hitSel);
	bitmap_layer_destroy(splash_layer);
	window_destroy(window);
}

void pedometer_update() {
	if (currX != 0 && currY != 0 && currZ != 0) {
		if (abs(abs(currX)-abs(lastX)) >= X_DELTA) {
			validX = true;
		}
		if (abs(abs(currY)-abs(lastY)) >= Y_DELTA) {
			validY = true;
		}
		if (abs(abs(currZ)-abs(lastZ)) >= Z_DELTA) {
			validZ = true;
		}
	}
}

void resetUpdate() {
	lastX = currX;
	lastY = currY;
	lastZ = currZ;
	validX = false;
	validY = false;
	validZ = false;
}

void update_ui_callback() {
	
	if (validX && validY && validZ) {
		pedometerCount++;
		static char buf[] = "123456890abcdefghijkl";
		snprintf(buf, sizeof(buf), "%d", pedometerCount);
		text_layer_set_text(pedCount, buf);
		layer_mark_dirty(window_get_root_layer(pedometer));
		if (stepGoal > 0 && pedometerCount == stepGoal){
			vibes_long_pulse();
			window_set_window_handlers(window, (WindowHandlers ) { .load = window_load,
																  .unload = window_unload, });
			window_stack_push(window, true);
		}
	}

	resetUpdate();
}

static void timer_callback(void *data) {
	AccelData accel = (AccelData ) { .x = 0, .y = 0, .z = 0 };
	accel_service_peek(&accel);

	if (!startedSession) {
		startedSession = true;
		lastX = accel.x;
		lastY = accel.y;
		lastZ = accel.z;
	} else {
		currX = accel.x;
		currY = accel.y;
		currZ = accel.z;
	}

	pedometer_update();
	update_ui_callback();
	
	layer_mark_dirty(window_get_root_layer(pedometer));
	timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}

void ped_load(Window *window) {
	steps = text_layer_create(GRect(0, 20, 150, 170));
	pedCount = text_layer_create(GRect(0, 80, 150, 170));

	if (isDark) {
		window_set_background_color(pedometer, GColorBlack);
		pedometerBack = gbitmap_create_with_resource(RESOURCE_ID_PED_WHITE);
		text_layer_set_background_color(steps, GColorClear);
		text_layer_set_text_color(steps, GColorWhite);
		text_layer_set_background_color(pedCount, GColorClear);
		text_layer_set_text_color(pedCount, GColorBlack);
	} else {
		window_set_background_color(pedometer, GColorWhite);
		pedometerBack = gbitmap_create_with_resource(RESOURCE_ID_PED_BLK);
		text_layer_set_background_color(steps, GColorClear);
		text_layer_set_text_color(steps, GColorBlack);
		text_layer_set_background_color(pedCount, GColorClear);
		text_layer_set_text_color(pedCount, GColorWhite);
	}
	
	pedometerBack_layer = bitmap_layer_create(GRect(0, 0, 145, 185));
	
	bitmap_layer_set_bitmap(pedometerBack_layer, pedometerBack);
	layer_add_child(window_get_root_layer(pedometer),
			bitmap_layer_get_layer(pedometerBack_layer));
	
	layer_add_child(window_get_root_layer(pedometer), (Layer*) steps);
	layer_add_child(window_get_root_layer(pedometer), (Layer*) pedCount);
	
	text_layer_set_font(pedCount, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ARLBD_30)));
	text_layer_set_text_alignment(pedCount, GTextAlignmentCenter);
	text_layer_set_text(steps, "                S T E P S");
	
	static char buf[] = "1234567890";
	snprintf(buf, sizeof(buf), "%d", pedometerCount);
	text_layer_set_text(pedCount, buf);
}

void ped_unload(Window *window) {
	window_destroy(pedometer);
	text_layer_destroy(steps);
	gbitmap_destroy(pedometerBack);
}

void start_callback(int index, void *ctx) {	
	menu_items[0].title = "Continue Run";
	menu_items[0].subtitle = "Ready for more?";
	layer_mark_dirty(simple_menu_layer_get_layer(pedometer_settings));

	pedometer = window_create();

	window_set_window_handlers(pedometer, (WindowHandlers ) { .load = ped_load,
					.unload = ped_unload, });

	window_stack_push(pedometer, true);
	timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}

void info_load(Window *window) {
	infor = text_layer_create(GRect(0, 0, 150, 150));

	if (isDark) {
		window_set_background_color(dev_info, GColorBlack);
		text_layer_set_background_color(infor, GColorClear);
		text_layer_set_text_color(infor, GColorWhite);
	} else {
		window_set_background_color(dev_info, GColorWhite);
		text_layer_set_background_color(infor, GColorClear);
		text_layer_set_text_color(infor, GColorBlack);
	}

	layer_add_child(window_get_root_layer(dev_info), (Layer*) infor);
	text_layer_set_text_alignment(infor, GTextAlignmentCenter);
	text_layer_set_text(infor,
			"\nDeveloped By: \nJathusan Thiruchelvanathan\n\nContact:\njathusan.t@gmail.com\n\n2014");
}

void info_unload(Window *window) {
	window_destroy(dev_info);
}

void info_callback(int index, void *ctx) {
	dev_info = window_create();

	window_set_window_handlers(dev_info, (WindowHandlers ) { .load = info_load,
					.unload = info_unload, });

	window_stack_push(dev_info, true);
}

void inc_click_handler(ClickRecognizerRef recognizer, void *context) {
	stepGoal += STEP_INCREMENT;
	static char buf[] = "123456";
	snprintf(buf, sizeof(buf), "%d", stepGoal);
	text_layer_set_text(stepGoalVisualizer, buf);

	if (stepGoal > 99900) {
		text_layer_set_font(stepGoalVisualizer,
				fonts_load_custom_font(
						resource_get_handle(RESOURCE_ID_ARLBD_25)));
	} else {
		text_layer_set_font(stepGoalVisualizer,
				fonts_load_custom_font(
						resource_get_handle(RESOURCE_ID_ARLBD_30)));
	}

	if (stepGoal != 0) {
		menu_items[1].subtitle = buf;
	} else {
		menu_items[1].subtitle = "Not Set";
	}
	layer_mark_dirty(simple_menu_layer_get_layer(pedometer_settings));
}

void dec_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (stepGoal >= STEP_INCREMENT) {
		stepGoal -= STEP_INCREMENT;
		static char buf[] = "123456";
		snprintf(buf, sizeof(buf), "%d", stepGoal);
		text_layer_set_text(stepGoalVisualizer, buf);

		if (stepGoal > 99900) {
			text_layer_set_font(stepGoalVisualizer,
					fonts_load_custom_font(
							resource_get_handle(RESOURCE_ID_ARLBD_25)));
		} else {
			text_layer_set_font(stepGoalVisualizer,
					fonts_load_custom_font(
							resource_get_handle(RESOURCE_ID_ARLBD_30)));
		}

		if (stepGoal != 0) {
			menu_items[1].subtitle = buf;
		} else {
			menu_items[1].subtitle = "Not Set";
		}
		layer_mark_dirty(simple_menu_layer_get_layer(pedometer_settings));
	}
}

void set_click_handler(ClickRecognizerRef recognizer, void *context) {
	window_stack_pop(true);
}

void goal_set_click_config(void *context) {
	window_single_click_subscribe(BUTTON_ID_DOWN,
			(ClickHandler) dec_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP,
			(ClickHandler) inc_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT, set_click_handler);
}

void stepGoal_load(Window *window) {
	stepGoalSetter = action_bar_layer_create();

	action_bar_layer_add_to_window(stepGoalSetter, set_stepGoal);
	action_bar_layer_set_click_config_provider(stepGoalSetter,
			goal_set_click_config);

	btn_dwn = gbitmap_create_with_resource(RESOURCE_ID_BTN_DOWN);
	btn_up = gbitmap_create_with_resource(RESOURCE_ID_BTN_UP);
	btn_sel = gbitmap_create_with_resource(RESOURCE_ID_BTN_SETUP);

	action_bar_layer_set_icon(stepGoalSetter, BUTTON_ID_UP, btn_up);
	action_bar_layer_set_icon(stepGoalSetter, BUTTON_ID_DOWN, btn_dwn);
	action_bar_layer_set_icon(stepGoalSetter, BUTTON_ID_SELECT, btn_sel);

	stepGoalVisualizer = text_layer_create(GRect(10, 50, 150, 150));
	text_layer_set_background_color(stepGoalVisualizer, GColorClear);
	text_layer_set_font(stepGoalVisualizer,
			fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ARLBD_30)));
	layer_add_child(window_get_root_layer(window), (Layer*) stepGoalVisualizer);

	static char buf[] = "123456";
	snprintf(buf, sizeof(buf), "%d", stepGoal);
	text_layer_set_text(stepGoalVisualizer, buf);

	if (isDark) {
		window_set_background_color(set_stepGoal, GColorBlack);
		text_layer_set_background_color(stepGoalVisualizer, GColorClear);
		text_layer_set_text_color(stepGoalVisualizer, GColorWhite);
		action_bar_layer_set_background_color(stepGoalSetter, GColorWhite);
	} else {
		window_set_background_color(set_stepGoal, GColorWhite);
		text_layer_set_background_color(stepGoalVisualizer, GColorClear);
		text_layer_set_text_color(stepGoalVisualizer, GColorBlack);
		action_bar_layer_set_background_color(stepGoalSetter, GColorBlack);
	}
}

void stepGoal_unload(Window *window) {
	action_bar_layer_destroy(stepGoalSetter);
	gbitmap_destroy(btn_up);
	gbitmap_destroy(btn_dwn);
	gbitmap_destroy(btn_sel);
	text_layer_destroy(stepGoalVisualizer);
}

void stepGoal_callback(int index, void *ctx) {
	set_stepGoal = window_create();

	window_set_window_handlers(set_stepGoal, (WindowHandlers ) { .load =
					stepGoal_load, .unload = stepGoal_unload, });

	window_stack_push(set_stepGoal, true);

	static char buf[] = "1234567890";
	snprintf(buf, sizeof(buf), "%d", stepGoal);

	//marking the layer as dirty, to acknowledge layer change
	if (stepGoal != 0) {
		menu_items[1].subtitle = buf;
	}
	layer_mark_dirty(simple_menu_layer_get_layer(pedometer_settings));
}

void theme_callback(int index, void *ctx) {
	if (isDark) {
		isDark = false;
		theme = "Light";
	} else {
		isDark = true;
		theme = "Dark";
	}

	char* new_string;
	new_string = malloc(strlen(theme) + 10);
	strcpy(new_string, "Current: ");
	strcat(new_string, theme);
	menu_items[3].subtitle = new_string;

	//marking the layer as dirty, to acknowledge layer change
	layer_mark_dirty(simple_menu_layer_get_layer(pedometer_settings));
}

////////////////////////////////////////////////////////////////////
// Settings Menu
///////////////////////////////////////////////////////////////////

void setup_menu_items() {
	if (!isDark) {
		theme = "Current: Light";
	} else {
		theme = "Current: Dark";
	}
	
	static char buf[] = "1234567890abcdefg";
	snprintf(buf, sizeof(buf), "%d in Total", totalSteps);

	for (int i = 0; i < (int) (sizeof(item_names) / sizeof(item_names[0]));
			i++) {
		menu_items[i] = (SimpleMenuItem ) { 
			.title = item_names[i], 
			.subtitle =	item_sub[i],};

		//Setting Callbacks
		if (i == 0) {
			menu_items[i].callback = start_callback;
		} else if (i == 1) {
			menu_items[i].callback = stepGoal_callback;
		} else if (i == 2) {
			menu_items[i].subtitle = buf;
		} else if (i == 3) {
			menu_items[i].subtitle = theme;
			menu_items[i].callback = theme_callback;
		} else if (i == 4 || i == 5) {
			menu_items[i].callback = info_callback;
		}
	}
}

void setup_menu_sections() {
	menu_sections[0] = (SimpleMenuSection ) { .items = menu_items, .num_items =
					sizeof(menu_items) / sizeof(menu_items[0]) };
}

void setup_menu_window() {
	menu_window = window_create();

	window_set_window_handlers(menu_window, (WindowHandlers ) { .load =
					settings_load, .unload = settings_unload, });
}

void settings_load(Window *window) {	
	Layer *layer = window_get_root_layer(menu_window);
	statusBar = gbitmap_create_with_resource(RESOURCE_ID_STATUS_BAR);

	pedometer_settings = simple_menu_layer_create(layer_get_bounds(layer),
			menu_window, menu_sections, 1, NULL);
	simple_menu_layer_set_selected_index(pedometer_settings, 0, true);
	layer_add_child(layer, simple_menu_layer_get_layer(pedometer_settings));
	window_set_status_bar_icon(menu_window, statusBar);
}

void settings_unload(Window *window) {
	layer_destroy(window_get_root_layer(menu_window));
	simple_menu_layer_destroy(pedometer_settings);
}

//Initializer/////////////////////////////////////////////////////////////////

void handle_init(void) {
	totalSteps = persist_read_int(totalSteps_SAVE);
	isDark = persist_read_bool(saveIsDark);
	window = window_create();
	
	setup_menu_items();
	setup_menu_sections();
	setup_menu_window();
	window_stack_push(menu_window, true);

	//for accelerometer data
	accel_data_service_subscribe(0, NULL);
}

void handle_deinit(void) {
	totalSteps += pedometerCount;
	persist_write_int(totalSteps_SAVE, totalSteps);
	persist_write_bool(saveIsDark, isDark);
	accel_data_service_unsubscribe();
	window_destroy(window);
}
