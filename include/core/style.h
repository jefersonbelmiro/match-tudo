#pragma once

#include "raylib.h"

// #define COLOR_CYAN    GetColor(0x00DCFFFF)
#define COLOR_CYAN   (Color){0, 220, 255, 255} // #00DCFFFF
#define COLOR_ORANGE (Color){255, 220, 0, 255} // #FFDC00FF
#define COLOR_GREEN  (Color){0, 255, 220, 255} // #00FFDCFF
#define COLOR_WHITE   WHITE
#define COLOR_BLACK   BLACK
#define COLOR_RED     RED

#define COLOR_PRIMARY COLOR_CYAN
#define COLOR_PRIMARY_DARK_2 ColorBrightness(COLOR_PRIMARY, -0.2f)
#define COLOR_PRIMARY_DARK_3 ColorBrightness(COLOR_PRIMARY, -0.3f)
#define COLOR_PRIMARY_DARK_4 ColorBrightness(COLOR_PRIMARY, -0.4f)
#define COLOR_PRIMARY_DARK_5 ColorBrightness(COLOR_PRIMARY, -0.5f)
#define COLOR_PRIMARY_DARK_6 ColorBrightness(COLOR_PRIMARY, -0.6f)
#define COLOR_PRIMARY_DARK_7 ColorBrightness(COLOR_PRIMARY, -0.7f)
#define COLOR_PRIMARY_DARK_8 ColorBrightness(COLOR_PRIMARY, -0.8f)
#define COLOR_PRIMARY_DARK_9 ColorBrightness(COLOR_PRIMARY, -0.9f)

#define COLOR_SECONDARY COLOR_ORANGE
#define COLOR_SECONDARY_DARK_2 ColorBrightness(COLOR_SECONDARY, -0.2f)
#define COLOR_SECONDARY_DARK_3 ColorBrightness(COLOR_SECONDARY, -0.3f)
#define COLOR_SECONDARY_DARK_4 ColorBrightness(COLOR_SECONDARY, -0.4f)
#define COLOR_SECONDARY_DARK_5 ColorBrightness(COLOR_SECONDARY, -0.5f)
#define COLOR_SECONDARY_DARK_6 ColorBrightness(COLOR_SECONDARY, -0.6f)
#define COLOR_SECONDARY_DARK_7 ColorBrightness(COLOR_SECONDARY, -0.7f)
#define COLOR_SECONDARY_DARK_8 ColorBrightness(COLOR_SECONDARY, -0.8f)
#define COLOR_SECONDARY_DARK_9 ColorBrightness(COLOR_SECONDARY, -0.9f)



#define COLOR_GRID_HOVER    ((Color){  50,  50,  50, 100 })
#define COLOR_GRID_LINE     ((Color){  80,  80,  105, 255 })

#define UI_ALPHA_DISABLED 0.35f

#define UI_PADDING       10.0f
#define UI_GAP            8.0f
#define UI_FONT_SIZE_SM  20.0f
#define UI_FONT_SIZE_MD  32.0f
