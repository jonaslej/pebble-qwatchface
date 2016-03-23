#ifndef __STORAGE_H__
#define __STORAGE_H__

// Version of the persistent storage
#define PERSIST_VERSION 1

#define PERSIST_VERSION_KEY 0
#define PERSIST_W_TEMP 1  // Latest temperature
#define PERSIST_W_COND 2  // Latest weather conditions
#define PERSIST_W_CELC 3  // Use celcius
#define PERSIST_W_UPDATE 4 // Latest update of the weather

#define KEY_BG_COLOR 5
#define KEY_TM_COLOR 6
#define KEY_DT_COLOR 7
#define KEY_WD_COLOR 8
#define KEY_WC_COLOR 9
#define KEY_TP_COLOR 10

#define PERSIST_MAX_VALUE 4
#endif
