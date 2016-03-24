#include <pebble.h>
#include "layers.h"
#include "storage.h"

static TextLayer *s_time_layer, *s_weather_layer, *s_temp_layer, *s_ampm_layer,
                  *s_icon_layer, *s_date_layer, *s_day_layer;
static GFont s_time_font, s_weather_font, s_temperature_font, s_ampm_font,
                  s_icon_font, s_date_font;
static GColor bgcolor, tmcolor, dtcolor, wdcolor, wccolor, tpcolor;
static char time_text[8];
static char temp_text[8];
static char cond_text[4];
static char date_text[12];
static char wday_text[12];

void update_temperature_text(char *text) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Weather Temperature Text");
  strcpy(temp_text, text);
  text_layer_set_text(s_temp_layer, temp_text);
}

void update_conditions_text(char *text) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Weather Conditions Text");
  strcpy(cond_text, text);
  text_layer_set_text(s_weather_layer, cond_text);
}

void update_icons(bool battery_charging, int battery_level, bool bt_connected) {
  static char s_iconbuffer[16];
  static char s_batt_icon[8];
  static char s_bt_icon[8];
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Icons, Battery State: %d, Bluetooth connected: %d", battery_level, bt_connected);

  // Select battery icon
  if(battery_charging) {
    snprintf(s_batt_icon, sizeof(s_batt_icon), "%s", "\ue800"); // battery-charging
  }
  else if(battery_level > 80) {
    snprintf(s_batt_icon, sizeof(s_batt_icon), "%s", "\ue804"); // battery-full
  }
  else if(battery_level > 50) {
    snprintf(s_batt_icon, sizeof(s_batt_icon), "%s", "\ue803"); // battery-high
  }
  else if(battery_level > 20) {
    snprintf(s_batt_icon, sizeof(s_batt_icon), "%s", "\ue802"); // battery-mid
  }
  else { // < 20
    snprintf(s_batt_icon, sizeof(s_batt_icon), "%s", "\ue801"); // battery-low
  }

  // Select Bluetooth icon
  snprintf(s_bt_icon, sizeof(s_bt_icon), "%s", bt_connected ? "\ue900" : "\ue901");

  // Combine icons
  snprintf(s_iconbuffer, sizeof(s_iconbuffer), "%s    %s", s_batt_icon, s_bt_icon);
  // Update text layer
  text_layer_set_text(s_icon_layer, s_iconbuffer);
}

void update_time() {
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

  static char s_day_buffer[16];
  // Write date into buffer
  strftime(s_day_buffer, sizeof(s_day_buffer), "%A", tick_time);

  // Display date on the TextLayer
  text_layer_set_text(s_day_layer, s_day_buffer);
  APP_LOG(APP_LOG_LEVEL_INFO, "Updated time and date: %s %s, %s %s", s_timebuffer, s_ampmbuffer, s_day_buffer, s_date_buffer);
}

void init_text_layer(TextLayer* v_text_layer, GColor v_text_color, GTextAlignment v_text_alignment, GFont* v_font) {
  //Set the colors
  text_layer_set_background_color(v_text_layer, GColorClear);
  text_layer_set_text_color(v_text_layer, v_text_color);

  //Set the text and textproperties
  text_layer_set_text_alignment(v_text_layer, v_text_alignment);

  // Apply font to TextLayer
  text_layer_set_font(v_text_layer, *v_font);
}

void load_fonts() {
  // Create GFonts
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTOLIGHT_44));
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_WEATHERFONT_22));
  s_temperature_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTOLIGHT_18));
  s_ampm_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTOTHIN_16));
  s_icon_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ICOMOON_22));
  s_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ROBOTOTHIN_22));
}

void create_text_layers(Window *window) {
  load_colors();
  // Already set the color of the background window to prevent it "flashing"
  window_set_background_color(window, bgcolor);
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  int center = bounds.size.h/2;
  int middle = bounds.size.w/2;
  APP_LOG(APP_LOG_LEVEL_INFO, "window bounds: W = %d, H = %d, Center: %d, Middle: %d", bounds.size.w, bounds.size.h, center, middle);

  // Create TextLayers
  s_time_layer = text_layer_create(GRect(0, center - 30, bounds.size.w - 30, 50));
  init_text_layer(s_time_layer, tmcolor, GTextAlignmentRight, &s_time_font);
  s_weather_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(8, 2), bounds.size.w, 30));
  init_text_layer(s_weather_layer, wccolor, GTextAlignmentCenter, &s_weather_font);
  s_icon_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(8, 2), bounds.size.w, 30));
  init_text_layer(s_icon_layer, wccolor, GTextAlignmentCenter, &s_icon_font);
  s_temp_layer = text_layer_create(GRect(bounds.size.w - 28, center - 4, 30, 25));
  init_text_layer(s_temp_layer, tpcolor, GTextAlignmentLeft, &s_temperature_font);
  s_ampm_layer = text_layer_create(GRect(bounds.size.w - 28, center - 24, 30, 25));
  init_text_layer(s_ampm_layer, tmcolor, GTextAlignmentLeft, &s_ampm_font);
  s_date_layer = text_layer_create(GRect(0, bounds.size.h - 40, bounds.size.w, 30));
  init_text_layer(s_date_layer, dtcolor, GTextAlignmentCenter, &s_date_font);
  s_day_layer = text_layer_create(GRect(0, bounds.size.h - 55, bounds.size.w, 30));
  init_text_layer(s_day_layer, wdcolor, GTextAlignmentCenter, &s_ampm_font);

  // Add layers as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_icon_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_temp_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_ampm_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_day_layer));
}
void unload_fonts() {
  // Unload GFonts
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_weather_font);
  fonts_unload_custom_font(s_temperature_font);
  fonts_unload_custom_font(s_ampm_font);
  fonts_unload_custom_font(s_icon_font);
  fonts_unload_custom_font(s_date_font);
}
void destroy_text_layers() {
  // Destroy TextLayers
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_temp_layer);
  text_layer_destroy(s_ampm_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_day_layer);
}

void load_colors() {
  APP_LOG(APP_LOG_LEVEL_INFO, "Loading colors");
  bgcolor = persist_exists(KEY_BG_COLOR) ? GColorFromHEX(persist_read_int(KEY_BG_COLOR)) : GColorBlack;
  tmcolor = persist_exists(KEY_TM_COLOR) ? GColorFromHEX(persist_read_int(KEY_TM_COLOR)) : GColorYellow;
  dtcolor = persist_exists(KEY_DT_COLOR) ? GColorFromHEX(persist_read_int(KEY_DT_COLOR)) : GColorWhite;
  wdcolor = persist_exists(KEY_WD_COLOR) ? GColorFromHEX(persist_read_int(KEY_WD_COLOR)) : GColorGreen;
  wccolor = persist_exists(KEY_WC_COLOR) ? GColorFromHEX(persist_read_int(KEY_WC_COLOR)) : GColorWhite;
  tpcolor = persist_exists(KEY_TP_COLOR) ? GColorFromHEX(persist_read_int(KEY_TP_COLOR)) : GColorWhite;
}

void set_colors(Window *window) {
  load_colors();
  window_set_background_color(window, bgcolor);
  text_layer_set_text_color(s_time_layer, tmcolor);
  text_layer_set_text_color(s_date_layer, dtcolor);
  text_layer_set_text_color(s_day_layer, wdcolor);
  text_layer_set_text_color(s_weather_layer, wccolor);
  text_layer_set_text_color(s_icon_layer, wccolor);
  text_layer_set_text_color(s_temp_layer, tpcolor);
}
