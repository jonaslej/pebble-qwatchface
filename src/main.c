#include <pebble.h>
#define KEY_TEMPERATURE 1
#define KEY_CONDITIONS 2
#define AppKeyJSReady 0
static Window *s_main_window;
static TextLayer *s_time_layer, *s_weather_layer, *s_temp_layer, *s_ampm_layer,
                  *s_icon_layer, *s_date_layer, *s_day_layer;

static GFont s_time_font, s_weather_font, s_temperature_font, s_ampm_font,
                  s_icon_font, s_date_font;

static bool s_js_ready, s_battery_charging, s_connected;
static int s_battery_level;

static void update_icons() {
  static char s_iconbuffer[16];
  static char s_batt_icon[8];
  static char s_bt_icon[8];
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Icons, Battery State: %d, Bluetooth connected: %d", s_battery_level, s_connected);

  // Select battery icon
  if(s_battery_charging) {
    snprintf(s_batt_icon, sizeof(s_batt_icon), "%s", "\ue800"); // battery-charging
  }
  else if(s_battery_level > 80) {
    snprintf(s_batt_icon, sizeof(s_batt_icon), "%s", "\ue804"); // battery-full
  }
  else if(s_battery_level > 50) {
    snprintf(s_batt_icon, sizeof(s_batt_icon), "%s", "\ue803"); // battery-high
  }
  else if(s_battery_level > 20) {
    snprintf(s_batt_icon, sizeof(s_batt_icon), "%s", "\ue802"); // battery-mid
  }
  else { // < 20
    snprintf(s_batt_icon, sizeof(s_batt_icon), "%s", "\ue801"); // battery-low
  }

  // Select Bluetooth icon
  snprintf(s_bt_icon, sizeof(s_bt_icon), "%s", s_connected ? "\ue900" : "\ue901");

  // Combine icons
  snprintf(s_iconbuffer, sizeof(s_iconbuffer), "%s   %s", s_batt_icon, s_bt_icon);
  // Update text layer
  text_layer_set_text(s_icon_layer, s_iconbuffer);
}
static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_charging = state.is_charging;
  s_battery_level = state.charge_percent;
  APP_LOG(APP_LOG_LEVEL_INFO, "Battery charge level: %d", s_battery_level);
  update_icons();
}

static void bluetooth_callback(bool connected) {
  s_connected = connected;
  update_icons();
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_timebuffer[8];
  strftime(s_timebuffer, sizeof(s_timebuffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_timebuffer);

  static char s_ampmbuffer[3];
  // Write am/pm into string buffer if not 24h clock
  strftime(s_ampmbuffer, sizeof(s_ampmbuffer), clock_is_24h_style() ?
                                            "" : "%P", tick_time);

  // Display am/pm on the TextLayer
  text_layer_set_text(s_ampm_layer, s_ampmbuffer);

  static char s_date_buffer[8];
  // Write date into buffer
  strftime(s_date_buffer, sizeof(s_date_buffer), "%b %d", tick_time);

  // Display date on the TextLayer
  text_layer_set_text(s_date_layer, s_date_buffer);

  static char s_day_buffer[8];
  // Write date into buffer
  strftime(s_day_buffer, sizeof(s_day_buffer), "%A", tick_time);

  // Display date on the TextLayer
  text_layer_set_text(s_day_layer, s_day_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
    APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send ??");
    // Send the message!
    if(s_js_ready) {
      app_message_outbox_send();
      APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send ??");
    }
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox received callback!");
  Tuple *ready_tuple = dict_find(iterator, AppKeyJSReady);
  if(ready_tuple) {
    // PebbleKit JS is ready! Safe to send messages
    s_js_ready = true;
    APP_LOG(APP_LOG_LEVEL_INFO, "Pebble-JS is ready");
  }
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[8];

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);

  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);

    // Assemble full string and display
    text_layer_set_text(s_weather_layer, conditions_buffer);
    text_layer_set_text(s_temp_layer, temperature_buffer);
  }
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


static void init_text_layer(TextLayer* v_text_layer,
      GColor v_background, GColor v_text_color,
      GTextAlignment v_text_alignment, GFont* v_font, char * text) {
  //Set the colors
  text_layer_set_background_color(v_text_layer, v_background);
  text_layer_set_text_color(v_text_layer, v_text_color);

  //Set the text and textproperties
  text_layer_set_text(v_text_layer, text);
  text_layer_set_text_alignment(v_text_layer, v_text_alignment);

  // Apply font to TextLayer
  text_layer_set_font(v_text_layer, *v_font);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  window_set_background_color(window, GColorBlack);
  int center = bounds.size.h/2;
  int middle = bounds.size.w/2;
  APP_LOG(APP_LOG_LEVEL_INFO, "window bounds: W = %d, H = %d, Center: %d, Middle: %d", bounds.size.w, bounds.size.h, center, middle);

  // Create GFonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTOLIGHT_44));
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_WEATHERFONT_22));
  s_temperature_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTOLIGHT_18));
  s_ampm_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTOTHIN_16));
  s_icon_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ICOMOON_22));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTOTHIN_22));

  // Create TextLayers
  s_time_layer = text_layer_create(GRect(0, center - 30, bounds.size.w - 30, 50));
  init_text_layer(s_time_layer, GColorClear, GColorYellow, GTextAlignmentRight, &s_time_font, "00:00");
  GSize textsize = text_layer_get_content_size(s_time_layer);
  APP_LOG(APP_LOG_LEVEL_INFO, "Time size: W = %d, H = %d", textsize.w, textsize.h);
  s_weather_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(8, 2), bounds.size.w, 30));
  init_text_layer(s_weather_layer, GColorClear, GColorWhite, GTextAlignmentCenter, &s_weather_font, "\uf00d");
  s_icon_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(8, 2), bounds.size.w, 30));
  init_text_layer(s_icon_layer, GColorClear, GColorWhite, GTextAlignmentCenter, &s_icon_font, "\ue803   \ue900");
  APP_LOG(APP_LOG_LEVEL_INFO, "sizeof stringbuffer: %d", sizeof("\ue803   \ue900"));
  s_temp_layer = text_layer_create(GRect(bounds.size.w - 28, center - 4, 30, 25));
  init_text_layer(s_temp_layer, GColorClear, GColorWhite, GTextAlignmentLeft, &s_temperature_font, "00°");
  s_ampm_layer = text_layer_create(GRect(bounds.size.w - 28, center - 24, 30, 25));
  init_text_layer(s_ampm_layer, GColorClear, GColorYellow, GTextAlignmentLeft, &s_ampm_font, "am");
  s_date_layer = text_layer_create(GRect(0, bounds.size.h - 40, bounds.size.w, 30));
  init_text_layer(s_date_layer, GColorClear, GColorWhite, GTextAlignmentCenter, &s_date_font, "Mnt 00");
  s_day_layer = text_layer_create(GRect(0, bounds.size.h - 55, bounds.size.w, 30));
  init_text_layer(s_day_layer, GColorClear, GColorGreen, GTextAlignmentCenter, &s_ampm_font, "Weekday");

  // Add layers as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_icon_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_temp_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_ampm_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_temp_layer);
  text_layer_destroy(s_ampm_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
  // Unload GFonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_weather_font);
  fonts_unload_custom_font(s_temperature_font);
  fonts_unload_custom_font(s_ampm_font);
  fonts_unload_custom_font(s_icon_font);
  fonts_unload_custom_font(s_date_font);
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

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
