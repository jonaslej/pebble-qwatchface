#include <pebble.h>
#include "weather.h"
#include "layers.h"
#include "storage.h"

static char* conditions_icons[] = {
  "\uf03e", //updating...
  "\uf00d", //clear sky - day
  "\uf002", //few clouds - day
  "\uf013", //scattered clouds - day
  "\uf009", //shower rain - day
  "\uf008", //rain - day
  "\uf010", //thunderstorm - day
  "\uf00a", //snow - day
  "\uf003", //mist - day
  "\uf086",
  "\uf013",
  "\uf029",
  "\uf028",
  "\uf02d",
  "\uf02a",
  "\uf02e",
  "\uf04a"  //mist - night
};

void update_weather() {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, PERSIST_W_CELC, persist_exists(PERSIST_W_CELC) ? (int)persist_read_bool(PERSIST_W_CELC) : 1);
  // Send the message!
  app_message_outbox_send();
  APP_LOG(APP_LOG_LEVEL_INFO, "Requested new weather info");
}

void update_weather_values(int temperature, int conditions) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Update Weather Values: %d %d", temperature, conditions);
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[8];
  // Assemble full string and display
  snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°", temperature);
  snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_icons[conditions]);
  update_temperature_text(temperature_buffer);
  update_conditions_text(conditions_buffer);
}

void store_weather_values(int temperature, int conditions) {
  persist_write_int(PERSIST_W_TEMP, temperature);
  persist_write_int(PERSIST_W_COND, conditions);
  persist_write_int(PERSIST_W_UPDATE, (int)time(NULL));
}

void show_weather_values(bool js_ready) {
  int temperature, conditions;
  // Check if weathervalues have been stored and how old they are
  if(persist_exists(PERSIST_W_UPDATE) && persist_read_int(PERSIST_W_UPDATE) > (time(NULL) - 3600)) {
    temperature = (persist_exists(PERSIST_W_TEMP) ? persist_read_int(PERSIST_W_TEMP) : 0);
    conditions = (persist_exists(PERSIST_W_COND) ? persist_read_int(PERSIST_W_COND) : 0);
    APP_LOG(APP_LOG_LEVEL_INFO, "Loaded weather info from storage. %d", (int)time(NULL));
  } else {
    temperature = 0;
    conditions = 0;
    if(js_ready) { update_weather(); }
    APP_LOG(APP_LOG_LEVEL_INFO, "No weather in storage or outdated. %d", (int)time(NULL));
  }
  update_weather_values(temperature, conditions);
}
