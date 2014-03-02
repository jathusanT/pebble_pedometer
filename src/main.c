/*
 * pebble pedometer
 * @author jathusant
 */

#include <pebble.h>
#include <run.h>

int main(void) {
    handle_init();
    app_event_loop();
    handle_deinit();
}