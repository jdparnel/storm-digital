#include "screens.h"
#include "ft813.h"
#include <stdio.h>
#include <stdbool.h>

// Draw interactive demo with button that responds to touch
void screen_touch_demo(const ft813_touch_t *inputs)
{
    static uint32_t press_count = 0;
    static uint8_t prev_tag = 0;
    bool button_pressed = false;
    
    // Edge detection: increment counter only on button press (not while held)
    // Tag 100 is assigned to the button, so only count when tag is exactly 100
    if (inputs && inputs->touching && inputs->tag == 100) {
        button_pressed = true;
        // Only increment when tag changes from not-100 to 100 (new press)
        if (prev_tag != 100) {
            press_count++;
        }
        prev_tag = 100;
    } else {
        prev_tag = inputs ? inputs->tag : 0;
    }
    
    // GD2 pattern: NO cmd_dlstart at beginning (it's done at end of previous swap)
    // Start with clear commands only
    ft813_cmd32(0x02FFFFFF);  // CLEAR_COLOR_RGB(255,255,255) - White background
    ft813_cmd32(0x26000007);  // CLEAR(1,1,1) - clear color, stencil, tag buffers
    
    // Title
    ft813_cmd32(0x04000000);  // COLOR_RGB(0,0,0) - Black text
    ft813_cmd_text(400, 60, 31, FT813_OPT_CENTER, "Touch Input Demo");
    
    // Touch status
    char buf[64];
    if (inputs && inputs->touching) {
        ft813_cmd32(0x0400FF00);  // COLOR_RGB(0,255,0) - Green text
        snprintf(buf, sizeof(buf), "Touch: X=%d Y=%d", inputs->x, inputs->y);
        ft813_cmd_text(400, 130, 28, FT813_OPT_CENTER, buf);
        
        ft813_cmd32(0x04FF8000);  // COLOR_RGB(255,128,0) - Orange text
        snprintf(buf, sizeof(buf), "Tag: %u", inputs->tag);
        ft813_cmd_text(400, 170, 28, FT813_OPT_CENTER, buf);
    } else {
        ft813_cmd32(0x04808080);  // COLOR_RGB(128,128,128) - Gray text
        ft813_cmd_text(400, 130, 28, FT813_OPT_CENTER, "Not touching");
        ft813_cmd_text(400, 170, 28, FT813_OPT_CENTER, "Tag: 0");
    }
    
    // Press counter
    ft813_cmd32(0x04000000);  // COLOR_RGB(0,0,0) - Black text
    snprintf(buf, sizeof(buf), "Press Count: %lu", press_count);
    ft813_cmd_text(400, 220, 28, FT813_OPT_CENTER, buf);
    
    // Draw button with tag assignment
    // Tag must be set BEFORE the button command
    ft813_cmd_tag(100);  // Assign tag 100 to the button
    
    // For buttons, we'll use options to change appearance
    // Draw button - colors are controlled by the coprocessor defaults
    ft813_cmd_button(300, 300, 200, 80, 29, button_pressed ? FT813_OPT_FLAT : 0, "Press Me!");
    
    // Reset tag to 0 (no tag for subsequent objects)
    ft813_cmd_tag(0);
    
    // Instructions
    ft813_cmd32(0x04606060);  // COLOR_RGB(96,96,96) - Dark gray
    ft813_cmd_text(400, 420, 26, FT813_OPT_CENTER, "Touch the button to increment counter");
    
    // GD2 swap pattern: DISPLAY → CMD_SWAP → CMD_LOADIDENTITY → CMD_DLSTART → flush
    ft813_swap();
}
