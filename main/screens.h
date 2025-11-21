#ifndef SCREENS_H
#define SCREENS_H

#include "ft813.h"

// Screen entrypoints (thin wrappers around driver-render functions)
void screen_touch_demo(const ft813_touch_t *inputs);

#endif // SCREENS_H
