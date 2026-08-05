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

#define COLOR_SECONDARY COLOR_GREEN
#define COLOR_SECONDARY_DARK_2 ColorBrightness(COLOR_SECONDARY, -0.2f)
#define COLOR_SECONDARY_DARK_3 ColorBrightness(COLOR_SECONDARY, -0.3f)
#define COLOR_SECONDARY_DARK_4 ColorBrightness(COLOR_SECONDARY, -0.4f)
#define COLOR_SECONDARY_DARK_5 ColorBrightness(COLOR_SECONDARY, -0.5f)
#define COLOR_SECONDARY_DARK_6 ColorBrightness(COLOR_SECONDARY, -0.6f)
#define COLOR_SECONDARY_DARK_7 ColorBrightness(COLOR_SECONDARY, -0.7f)
#define COLOR_SECONDARY_DARK_8 ColorBrightness(COLOR_SECONDARY, -0.8f)
#define COLOR_SECONDARY_DARK_9 ColorBrightness(COLOR_SECONDARY, -0.9f)


// #define COLOR_GRID_HOVER    ((Color){  255,  255,  255, 185 }) 
// #define COLOR_GRID_LINE     ((Color){  0,  0,  0, 255 })
// #define COLOR_GRID_SELECTED ((Color){  255, 255, 255, 205 })

#define COLOR_GRID_HOVER    COLOR_PRIMARY
#define COLOR_GRID_LINE     ((Color){  0,  0,  0, 255 })
#define COLOR_GRID_SELECTED COLOR_PRIMARY
#define GRID_LINE_TICK 4.0

#define UI_ALPHA_DISABLED 0.35f

#define UI_PADDING       10.0f
#define UI_GAP            8.0f
#define UI_FONT_SIZE_SM  20.0f
#define UI_FONT_SIZE_MD  32.0f


// ---------------------------------------------------------------------------
// UI colors
// ---------------------------------------------------------------------------

#define UI_COLOR_HOVER           COLOR_WHITE
#define UI_COLOR_TEXT            ((Color) {220, 220, 220, 255})

#define UI_COLOR_PANEL_BG        ColorBrightness(COLOR_PRIMARY, -0.8f)
#define UI_COLOR_PANEL_BG_A(a)   ColorAlpha(ColorBrightness(COLOR_PRIMARY, -0.8f), (a))

#define UI_COLOR_WIDGET_BG       ColorBrightness(COLOR_PRIMARY, -0.6f)
#define UI_COLOR_WIDGET_FG       ColorBrightness(COLOR_PRIMARY, -0.3f)
#define UI_COLOR_WIDGET_HOVER    ColorBrightness(COLOR_PRIMARY, -0.45f)  // hovered state
#define UI_COLOR_WIDGET_ACTIVE   ColorBrightness(COLOR_PRIMARY,  0.4f)   // pressed/active

#define UI_COLOR_FLEET_PATH      ColorAlpha(ColorBrightness(COLOR_PRIMARY, -0.4f), 0.8)
#define UI_COLOR_FLEET_PATH_NODE ColorAlpha(ColorBrightness(COLOR_PRIMARY, -0.4f), 0.8)

// ---------------------------------------------------------------------------
// UI interaction alphas — used consistently across all interactive widgets
// ---------------------------------------------------------------------------

#define UI_ALPHA_IDLE     0.85f   // normal resting state
#define UI_ALPHA_HOVER    1.00f   // on hover
#define UI_ALPHA_ACTIVE   1.00f   // pressed / selected
#define UI_ALPHA_DISABLED 0.35f

// ---------------------------------------------------------------------------
// UI layout
// ---------------------------------------------------------------------------

#define UI_PADDING       10.0f
#define UI_GAP            8.0f
#define UI_FONT_SIZE_SM  20.0f
#define UI_FONT_SIZE_MD  32.0f

#define UI_PATH_NODE_SIZE 10
