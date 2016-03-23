#include <pebble.h>
#include "weather.h"
#include "layers.h"
#include "storage.h"
#define KEY_TEMPERATURE 1
#define KEY_CONDITIONS 2
#define AppKeyJSReady 0

static Window *s_main_window;
static bool s_js_ready, s_battery_charging, s_connected;
static int s_battery_level;

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_charging = state.is_charging;
  s_battery_level = state.charge_percent;
  APP_LOG(APP_LOG_LEVEL_INFO, "Battery charge level: %d", s_battery_level);
  update_icons(s_battery_charging, s_battery_level, s_connected);
}

static void bluetooth_callback(bool connected) {
  s_connected = connected;
  update_icons(s_battery_charging, s_battery_level, s_connected);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0 && s_js_ready) {
    update_weather();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received callback!");
  Tuple *ready_tuple = dict_find(iterator, AppKeyJSReady);
  if(ready_tuple) {
    // PebbleKit JS is ready! Safe to send messages
    s_js_ready = true;
    APP_LOG(APP_LOG_LEVEL_INFO, "Pebble-JS is ready");
    show_weather_values(s_js_ready);
  }

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);

  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    int temperature = (int)temp_tuple->value->int32;
    int conditions = (int)conditions_tuple->value->int32;
    update_weather_values(temperature, conditions);
    store_weather_values(temperature, conditions);
    APP_LOG(APP_LOG_LEVEL_INFO, "Callback contained weather info.");
  }

  Tuple *bgcolor = dict_find(iterator, KEY_BG_COLOR);
  if(bgcolor) {
    persist_write_int(KEY_BG_COLOR, bgcolor->value->int32);
  }
  Tuple *tmcolor = dict_find(iterator, KEY_TM_COLOR);
  if(tmcolor) {
    persist_write_int(KEY_TM_COLOR, tmcolor->value->int32);
  }
  Tuple *dtcolor = dict_find(iterator, KEY_DT_COLOR);
  if(dtcolor) {
    persist_write_int(KEY_DT_COLOR, dtcolor->value->int32);
  }
  Tuple *wdcolor = dict_find(iterator, KEY_WD_COLOR);
  if(wdcolor) {
    persist_write_int(KEY_WD_COLOR, wdcolor->value->int32);
  }
  Tuple *wccolor = dict_find(iterator, KEY_WC_COLOR);
  if(wccolor) {
    persist_write_int(KEY_WC_COLOR, wccolor->value->int32);
  }
  Tuple *tpcolor = dict_find(iterator, KEY_TP_COLOR);
  if(tpcolor) {
    persist_write_int(KEY_TP_COLOR, tpcolor->value->int32);
  }
  set_colors(s_main_window);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void main_window_load(Window *window) {
  load_fonts();
  create_text_layers(window);
}

static void main_window_unload(Window *window) {
  destroy_text_layers();
  unload_fonts();
}

static void migrate_storage_data(int current_version) {
  switch(current_version) {
    case 0:
      // No persistent storage existed
      break;
    default:
      // Unknown version, clear persistent data
      APP_LOG(APP_LOG_LEVEL_INFO, "Unknown storage data version, clear all persistent data");
      for(int i = 0; i <= PERSIST_MAX_VALUE; i++) {
        persist_delete(i);
      }
      break;
  }
  persist_write_int(PERSIST_VERSION_KEY, PERSIST_VERSION);
}

static void init() {
  setlocale(LC_TIME, "");
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Verify persistent storage
  int persist_version = 0;
  bool persist_uptodate = false;
  if(persist_exists(PERSIST_VERSION_KEY)) {
    persist_version = persist_read_int(PERSIST_VERSION_KEY);
    if(persist_version == PERSIST_VERSION) {
      persist_uptodate = true;
    }
  }
  if(!persist_uptodate) {
    migrate_storage_data(persist_version);
  }

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  // Ensure Bluetooth connection is correct from the start
  s_connected = connection_service_peek_pebblekit_connection();
  // Register for Battery callback
  battery_state_service_subscribe(battery_callback);
  // Ensure battery level is displayed from the start
  // This will also update the icon for the Bluetooth connection
  battery_callback(battery_state_service_peek());

  show_weather_values(false);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
