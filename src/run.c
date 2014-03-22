/*
 * pebble pedometer
 * @author jathusant
 */

#include <pebble.h>
#include <run.h>
#include <math.h>

// Total Steps (TS)
#define TS 1
// Total Steps Default (TSD)
#define TSD 1

static Window *window;
static Window *menu_window;
static Window *set_stepGoal;
static Window *pedometer;
static Window *dev_info;

static SimpleMenuLayer *pedometer_settings;
static SimpleMenuItem menu_items[8];
static SimpleMenuSection menu_sections[1];
ActionBarLayer *stepGoalSetter;

// Menu Item names and subtitles
char *item_names[8] = { "Start", "Step Goal", "Overall Steps",
		"Overall Calories", "Sensitivity", "Theme", "Version", "About" };
char *item_sub[8] = { "Lets Exercise!", "Not Set", "0 in Total", "0 Burned",
		"", "", "v1.4-RELEASE", "Jathusan T." };

// Timer used to determine next step check
static AppTimer *timer;

// Text Layers
TextLayer *main_message;
TextLayer *main_message2;
TextLayer *hitBack;
TextLayer *stepGoalVisualizer;
TextLayer *steps;
TextLayer *pedCount;
TextLayer *infor;
TextLayer *calories;
TextLayer *stepGoalText;

// Bitmap Layers
static GBitmap *btn_dwn;
static GBitmap *btn_up;
static GBitmap *btn_sel;
static GBitmap *statusBar;
GBitmap *pedometerBack;
BitmapLayer *pedometerBack_layer;
GBitmap *splash;
BitmapLayer *splash_layer;

// interval to check for next step (in ms)
const int ACCEL_STEP_MS = 475;
// value to auto adjust step acceptance 
const int PED_ADJUST = 2;
// steps required per calorie
const int STEPS_PER_CALORIE = 22;
// value by which step goal is incremented
const int STEP_INCREMENT = 50;
// values for max/min number of calibration options 
const int MAX_CALIBRATION_SETTINGS = 3;
const int MIN_CALIBRATION_SETTINGS = 1;

int X_DELTA = 35;
int Y_DELTA, Z_DELTA = 185;
int YZ_DELTA_MIN = 175;
int YZ_DELTA_MAX = 195; 
int X_DELTA_TEMP, Y_DELTA_TEMP, Z_DELTA_TEMP = 0;
int lastX, lastY, lastZ, currX, currY, currZ = 0;
int sensitivity = 1;

long stepGoal = 0;
long pedometerCount = 0;
long caloriesBurned = 0;
long tempTotal = 0;

bool did_pebble_vibrate = false;
bool validX, validY, validZ = false;
bool SID;
bool isDark;
bool startedSession = false;

// Strings used to display theme and calibration options
char *theme;
char *cal = "Regular Sensitivity";

// stores total steps since app install
static long totalSteps = TSD;

void start_callback(int index, void *ctx) {
	accel_data_service_subscribe(0, NULL);

	menu_items[0].title = "Continue Run";
	menu_items[0].subtitle = "Ready for more?";
	layer_mark_dirty(simple_menu_layer_get_layer(pedometer_settings));

	pedometer = window_create();

	window_set_window_handlers(pedometer, (WindowHandlers ) { .load = ped_load,
					.unload = ped_unload, });

	window_stack_push(pedometer, true);
	timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}

void info_callback(int index, void *ctx) {
	dev_info = window_create();

	window_set_window_handlers(dev_info, (WindowHandlers ) { .load = info_load,
					.unload = info_unload, });

	window_stack_push(dev_info, true);
}

void stepGoal_callback(int index, void *ctx) {
	set_stepGoal = window_create();

	window_set_window_handlers(set_stepGoal, (WindowHandlers ) { .load =
					stepGoal_load, .unload = stepGoal_unload, });

	window_stack_push(set_stepGoal, true);

	static char buf[] = "1234567890";
	snprintf(buf, sizeof(buf), "%ld", stepGoal);

	if (stepGoal != 0) {
		menu_items[1].subtitle = buf;
	}
	layer_mark_dirty(simple_menu_layer_get_layer(pedometer_settings));
}

void calibration_callback(int index, void *ctx) {
	
	if (sensitivity >= MIN_CALIBRATION_SETTINGS && sensitivity < MAX_CALIBRATION_SETTINGS){
		sensitivity++;
	} else if (sensitivity == MAX_CALIBRATION_SETTINGS) {
		sensitivity = MIN_CALIBRATION_SETTINGS;
	}

	cal = determineCal(sensitivity);
	
	menu_items[4].subtitle = cal;
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
	menu_items[5].subtitle = new_string;

	layer_mark_dirty(simple_menu_layer_get_layer(pedometer_settings));
}

char* determineCal(int cal){
	switch(cal){
		case 2:
		X_DELTA = 45;
		Y_DELTA = 235;
		Z_DELTA = 235;
		YZ_DELTA_MIN = 225;
		YZ_DELTA_MAX = 245; 
		return "Not Sensitive";
		case 3:
		X_DELTA = 25;
		Y_DELTA = 110;
		Z_DELTA = 110;
		YZ_DELTA_MIN = 100;
		YZ_DELTA_MAX = 120; 
		return "Very Sensitive";
		default:
		X_DELTA = 35;
		Y_DELTA = 185;
		Z_DELTA = 185;
		YZ_DELTA_MIN = 175;
		YZ_DELTA_MAX = 195; 
		return "Regular Sensitivity";
	}
}
void changeFontToFit() {
	if (stepGoal > 99900) {
		text_layer_set_font(stepGoalVisualizer,
				fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	} else {
		text_layer_set_font(stepGoalVisualizer,
				fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	}
}

void inc_click_handler(ClickRecognizerRef recognizer, void *context) {
	stepGoal += STEP_INCREMENT;
	static char buf[] = "123456";
	snprintf(buf, sizeof(buf), "%ld", stepGoal);
	text_layer_set_text(stepGoalVisualizer, buf);

	changeFontToFit();

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
		snprintf(buf, sizeof(buf), "%ld", stepGoal);
		text_layer_set_text(stepGoalVisualizer, buf);

		changeFontToFit();

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
	const uint16_t rep_interval = 50;

	window_single_repeating_click_subscribe(BUTTON_ID_DOWN, rep_interval,
			(ClickHandler) dec_click_handler);
	window_single_repeating_click_subscribe(BUTTON_ID_UP, rep_interval,
			(ClickHandler) inc_click_handler);
	window_single_click_subscribe(BUTTON_ID_SELECT,
			(ClickHandler) set_click_handler);
}

void setup_menu_items() {
	static char buf[] = "1234567890abcdefg";
	snprintf(buf, sizeof(buf), "%ld in Total", totalSteps);

	static char buf2[] = "1234567890abcdefg";
	snprintf(buf2, sizeof(buf2), "%ld Burned",
			(long) (totalSteps / STEPS_PER_CALORIE));

	for (int i = 0; i < (int) (sizeof(item_names) / sizeof(item_names[0]));
			i++) {
		menu_items[i] = (SimpleMenuItem ) { .title = item_names[i], .subtitle =
						item_sub[i], };

		//Setting Callbacks
		if (i == 0) {
			menu_items[i].callback = start_callback;
		} else if (i == 1) {
			menu_items[i].callback = stepGoal_callback;
		} else if (i == 2) {
			menu_items[i].subtitle = buf;
		} else if (i == 3) {
			menu_items[i].subtitle = buf2;
		} else if (i ==4){
			menu_items[i].subtitle = determineCal(sensitivity);
			menu_items[i].callback = calibration_callback;
		} else if (i == 5) {
			menu_items[i].subtitle = theme;
			menu_items[i].callback = theme_callback;
		} else if (i == 6 || i == 7) {
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

	stepGoalText = text_layer_create(GRect(5, 5, 150, 150));
	stepGoalVisualizer = text_layer_create(GRect(10, 50, 150, 150));
	text_layer_set_background_color(stepGoalVisualizer, GColorClear);
	text_layer_set_font(stepGoalVisualizer,
			fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_background_color(stepGoalText, GColorClear);
	text_layer_set_font(stepGoalText,
			fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
	layer_add_child(window_get_root_layer(set_stepGoal),
			(Layer*) stepGoalVisualizer);
	layer_add_child(window_get_root_layer(set_stepGoal), (Layer*) stepGoalText);

	static char buf[] = "123456";
	snprintf(buf, sizeof(buf), "%ld", stepGoal);
	text_layer_set_text(stepGoalVisualizer, buf);
	text_layer_set_text(stepGoalText, "Set Goal");

	if (isDark) {
		window_set_background_color(set_stepGoal, GColorBlack);
		text_layer_set_text_color(stepGoalVisualizer, GColorWhite);
		text_layer_set_text_color(stepGoalText, GColorWhite);
		action_bar_layer_set_background_color(stepGoalSetter, GColorWhite);
	} else {
		window_set_background_color(set_stepGoal, GColorWhite);
		text_layer_set_text_color(stepGoalVisualizer, GColorBlack);
		text_layer_set_text_color(stepGoalText, GColorBlack);
		action_bar_layer_set_background_color(stepGoalSetter, GColorBlack);
	}
}

void stepGoal_unload(Window *window) {
	window_destroy(set_stepGoal);
	action_bar_layer_destroy(stepGoalSetter);
	gbitmap_destroy(btn_up);
	gbitmap_destroy(btn_dwn);
	gbitmap_destroy(btn_sel);
	text_layer_destroy(stepGoalVisualizer);
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
	window_destroy(menu_window);
}

void ped_load(Window *window) {
	steps = text_layer_create(GRect(0, 20, 150, 170));
	pedCount = text_layer_create(GRect(0, 80, 150, 170));
	calories = text_layer_create(GRect(0, 10, 150, 170));

	if (isDark) {
		window_set_background_color(pedometer, GColorBlack);
		pedometerBack = gbitmap_create_with_resource(RESOURCE_ID_PED_WHITE);
		text_layer_set_background_color(steps, GColorClear);
		text_layer_set_text_color(steps, GColorBlack);
		text_layer_set_background_color(pedCount, GColorClear);
		text_layer_set_text_color(pedCount, GColorBlack);
		text_layer_set_background_color(calories, GColorClear);
		text_layer_set_text_color(calories, GColorWhite);
	} else {
		window_set_background_color(pedometer, GColorWhite);
		pedometerBack = gbitmap_create_with_resource(RESOURCE_ID_PED_BLK);
		text_layer_set_background_color(steps, GColorClear);
		text_layer_set_text_color(steps, GColorWhite);
		text_layer_set_background_color(pedCount, GColorClear);
		text_layer_set_text_color(pedCount, GColorWhite);
		text_layer_set_background_color(calories, GColorClear);
		text_layer_set_text_color(calories, GColorBlack);
	}

	pedometerBack_layer = bitmap_layer_create(GRect(0, 0, 145, 185));

	bitmap_layer_set_bitmap(pedometerBack_layer, pedometerBack);
	layer_add_child(window_get_root_layer(pedometer),
			bitmap_layer_get_layer(pedometerBack_layer));

	layer_add_child(window_get_root_layer(pedometer), (Layer*) steps);
	layer_add_child(window_get_root_layer(pedometer), (Layer*) pedCount);
	layer_add_child(window_get_root_layer(pedometer), (Layer*) calories);

	text_layer_set_font(pedCount,
			fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_font(calories,
			fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));

	text_layer_set_text_alignment(pedCount, GTextAlignmentCenter);
	text_layer_set_text_alignment(calories, GTextAlignmentCenter);

	text_layer_set_text(steps, "\n\n\n                S T E P S");

	static char buf[] = "1234567890";
	snprintf(buf, sizeof(buf), "%ld", pedometerCount);
	text_layer_set_text(pedCount, buf);

	static char buf2[] = "1234567890abcdefghijkl";
	snprintf(buf2, sizeof(buf2), "%ld Calories", caloriesBurned);
	text_layer_set_text(calories, buf2);
}

void ped_unload(Window *window) {
	app_timer_cancel(timer);
	window_destroy(pedometer);
	text_layer_destroy(pedCount);
	text_layer_destroy(calories);
	text_layer_destroy(steps);
	gbitmap_destroy(pedometerBack);
	accel_data_service_unsubscribe();
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
	text_layer_destroy(infor);
	window_destroy(dev_info);
}

void window_load(Window *window) {

	splash = gbitmap_create_with_resource(RESOURCE_ID_SPLASH);
	window_set_background_color(window, GColorBlack);

	splash_layer = bitmap_layer_create(GRect(0, 0, 145, 185));
	bitmap_layer_set_bitmap(splash_layer, splash);
	layer_add_child(window_get_root_layer(window),
			bitmap_layer_get_layer(splash_layer));

	main_message = text_layer_create(GRect(0, 0, 150, 170));
	main_message2 = text_layer_create(GRect(3, 35, 150, 170));
	hitBack = text_layer_create(GRect(3, 40, 200, 170));

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

	text_layer_set_background_color(hitBack, GColorClear);
	text_layer_set_text_color(hitBack, GColorWhite);
	text_layer_set_font(hitBack,
			fonts_load_custom_font(
					resource_get_handle(RESOURCE_ID_ROBOTO_LT_15)));
	layer_add_child(window_get_root_layer(window), (Layer*) hitBack);

	text_layer_set_text(main_message, "     GOAL");
	text_layer_set_text(main_message2, "          Reached!");
	text_layer_set_text(hitBack, "\n\n\n\n\n\n        Press Back...");
}

void window_unload(Window *window) {
	window_destroy(window);
	text_layer_destroy(main_message);
	text_layer_destroy(main_message2);
	text_layer_destroy(hitBack);
	bitmap_layer_destroy(splash_layer);
}

void autoCorrectZ(){
	if (Z_DELTA > YZ_DELTA_MAX){
		Z_DELTA = YZ_DELTA_MAX; 
	} else if (Z_DELTA < YZ_DELTA_MIN){
		Z_DELTA = YZ_DELTA_MIN;
	}
}

void autoCorrectY(){
	if (Y_DELTA > YZ_DELTA_MAX){
		Y_DELTA = YZ_DELTA_MAX; 
	} else if (Y_DELTA < YZ_DELTA_MIN){
		Y_DELTA = YZ_DELTA_MIN;
	}
}

void pedometer_update() {
	if (startedSession) {
		X_DELTA_TEMP = abs(abs(currX) - abs(lastX));
		if (X_DELTA_TEMP >= X_DELTA) {
			validX = true;
		}
		Y_DELTA_TEMP = abs(abs(currY) - abs(lastY));
		if (Y_DELTA_TEMP >= Y_DELTA) {
			validY = true;
			if (Y_DELTA_TEMP - Y_DELTA > 200){
				autoCorrectY();
				Y_DELTA = (Y_DELTA < YZ_DELTA_MAX) ? Y_DELTA + PED_ADJUST : Y_DELTA;
			} else if (Y_DELTA - Y_DELTA_TEMP > 175){
				autoCorrectY();
				Y_DELTA = (Y_DELTA > YZ_DELTA_MIN) ? Y_DELTA - PED_ADJUST : Y_DELTA;
			}
		}
		Z_DELTA_TEMP = abs(abs(currZ) - abs(lastZ));
		if (abs(abs(currZ) - abs(lastZ)) >= Z_DELTA) {
			validZ = true;
			if (Z_DELTA_TEMP - Z_DELTA > 200){
				autoCorrectZ();
				Z_DELTA = (Z_DELTA < YZ_DELTA_MAX) ? Z_DELTA + PED_ADJUST : Z_DELTA;
			} else if (Z_DELTA - Z_DELTA_TEMP > 175){
				autoCorrectZ();
				Z_DELTA = (Z_DELTA < YZ_DELTA_MAX) ? Z_DELTA + PED_ADJUST : Z_DELTA;
			}
		}
	} else {
		startedSession = true;
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
	if ((validX && validY && !did_pebble_vibrate) || (validX && validZ && !did_pebble_vibrate)) {
		pedometerCount++;
		tempTotal++;

		caloriesBurned = (int) (pedometerCount / STEPS_PER_CALORIE);
		static char calBuf[] = "123456890abcdefghijkl";
		snprintf(calBuf, sizeof(calBuf), "%ld Calories", caloriesBurned);
		text_layer_set_text(calories, calBuf);

		static char buf[] = "123456890abcdefghijkl";
		snprintf(buf, sizeof(buf), "%ld", pedometerCount);
		text_layer_set_text(pedCount, buf);

		static char buf2[] = "123456890abcdefghijkl";
		snprintf(buf2, sizeof(buf2), "%ld in Total", tempTotal);
		menu_items[2].subtitle = buf2;

		static char buf3[] = "1234567890abcdefg";
		snprintf(buf3, sizeof(buf3), "%ld Burned",
				(long) (tempTotal / STEPS_PER_CALORIE));
		menu_items[3].subtitle = buf3;

		layer_mark_dirty(window_get_root_layer(pedometer));
		layer_mark_dirty(window_get_root_layer(menu_window));

		if (stepGoal > 0 && pedometerCount == stepGoal) {
			vibes_long_pulse();
			window_set_window_handlers(window, (WindowHandlers ) { .load =
							window_load, .unload = window_unload, });
			window_stack_push(window, true);
		}
	}

	resetUpdate();
}

static void timer_callback(void *data) {
	AccelData accel = (AccelData ) { .x = 0, .y = 0, .z = 0 };
	accel_service_peek(&accel);

	if (!startedSession) {
		lastX = accel.x;
		lastY = accel.y;
		lastZ = accel.z;
	} else {
		currX = accel.x;
		currY = accel.y;
		currZ = accel.z;
	}
	
	did_pebble_vibrate = accel.did_vibrate;

	pedometer_update();
	update_ui_callback();

	layer_mark_dirty(window_get_root_layer(pedometer));
	timer = app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}

void handle_init(void) {
	tempTotal = totalSteps = persist_exists(TS) ? persist_read_int(TS) : TSD;
	isDark = persist_exists(SID) ? persist_read_bool(SID) : true;

	if (!isDark) {
		theme = "Current: Light";
	} else {
		theme = "Current: Dark";
	}

	window = window_create();

	setup_menu_items();
	setup_menu_sections();
	setup_menu_window();

	window_stack_push(menu_window, true);
}

void handle_deinit(void) {
	totalSteps += pedometerCount;
	persist_write_int(TS, totalSteps);
	persist_write_bool(SID, isDark);
	accel_data_service_unsubscribe();
	window_destroy(menu_window);
}
