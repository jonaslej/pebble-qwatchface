#include <pebble.h>
#include "weather.h"
#include "layers.h"

static char* conditions_icons[] = {
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


void update_weather(int temperature, int conditions) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[8];
  // Assemble full string and display
  snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°", temperature);
  snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_icons[conditions]);
  update_temperature_text(temperature_buffer);
  update_conditions_text(conditions_buffer);
}
