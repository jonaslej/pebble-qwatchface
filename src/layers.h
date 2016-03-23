#ifndef __LAYERS_H__
#define __LAYERS_H__

void init_text_layer(TextLayer*, GColor, GTextAlignment, GFont*);
void create_text_layers(Window*);
void destroy_text_layers();
void update_icons(bool, int, bool);
void update_time();
void update_temperature_text(char*);
void update_conditions_text(char *);
void load_fonts();
void unload_fonts();
void load_colors();
void set_colors();
#endif
