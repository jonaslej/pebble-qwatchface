#ifndef __LAYERS_H__
#define __LAYERS_H__

void init_text_layer(TextLayer*, GColor, GColor, GTextAlignment, GFont*, char*);
void create_text_layers(Window*);
void destroy_text_layers();
void update_icons(bool, int, bool);
void update_time();
void update_temperature_text(char*);
void update_conditions_text(char *);
#endif
