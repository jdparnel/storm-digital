#ifndef SCREENS_H
#define SCREENS_H

#include "ft813.h"

// Screen IDs for navigation
typedef enum {
    SCREEN_HOME,
    SCREEN_SECONDARY
} screen_id_t;

// Screen entrypoints
void screen_home(const ft813_touch_t *inputs);
void screen_secondary(const ft813_touch_t *inputs);

// Navigation API
void screens_set_current(screen_id_t screen);
screen_id_t screens_get_current(void);
void screens_render(const ft813_touch_t *inputs);

#endif // SCREENS_H
