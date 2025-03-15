#ifndef RENDER_H
#define RENDER_H

#include <main.h>

// defined in render.c
extern const char* g_rPixelColors[16];
extern const char* g_rTextColors[16];

// pixel color IDs
#define PXID_BLACK          (0)
#define PXID_RED            (1)
#define PXID_GREEN          (2)
#define PXID_YELLOW         (3)
#define PXID_BLUE           (4)
#define PXID_MAGENTA        (5)
#define PXID_CYAN           (6)
#define PXID_WHITE          (7)
#define PXID_BR_BLACK       (8)
#define PXID_BR_RED         (9)
#define PXID_BR_GREEN       (10)
#define PXID_BR_YELLOW      (11)
#define PXID_BR_BLUE        (12)
#define PXID_BR_MAGENTA     (13)
#define PXID_BR_CYAN        (14)
#define PXID_BR_WHITE       (15)
#define PXID_CLEAR          (16)

// pixel color string literals
#define PXL_BLACK           "\x1B[40m"
#define PXL_RED             "\x1B[41m"
#define PXL_GREEN           "\x1B[42m"
#define PXL_YELLOW          "\x1B[43m"
#define PXL_BLUE            "\x1B[44m"
#define PXL_MAGENTA         "\x1B[45m"
#define PXL_CYAN            "\x1B[46m"
#define PXL_WHITE           "\x1B[47m"
#define PXL_BR_BLACK        "\x1B[100m"
#define PXL_BR_RED          "\x1B[101m"
#define PXL_BR_GREEN        "\x1B[102m"
#define PXL_BR_YELLOW       "\x1B[103m"
#define PXL_BR_BLUE         "\x1B[104m"
#define PXL_BR_MAGENTA      "\x1B[105m"
#define PXL_BR_CYAN         "\x1B[106m"
#define PXL_BR_WHITE        "\x1B[107m"
#define PXL_RESET           "\x1B[0m\x1B[47m\x1B[30m"

// text color string literals
#define TXT_BLACK           "\x1B[30m"
#define TXT_RED             "\x1B[31m"
#define TXT_GREEN           "\x1B[32m"
#define TXT_YELLOW          "\x1B[33m"
#define TXT_BLUE            "\x1B[34m"
#define TXT_MAGENTA         "\x1B[35m"
#define TXT_CYAN            "\x1B[36m"
#define TXT_WHITE           "\x1B[37m"
#define TXT_BR_BLACK        "\x1B[90m"
#define TXT_BR_RED          "\x1B[91m"
#define TXT_BR_GREEN        "\x1B[92m"
#define TXT_BR_YELLOW       "\x1B[93m"
#define TXT_BR_BLUE         "\x1B[94m"
#define TXT_BR_MAGENTA      "\x1B[95m"
#define TXT_BR_CYAN         "\x1B[96m"
#define TXT_BR_WHITE        "\x1B[97m"
#define TXT_RESET           "\x1B[0m\x1B[47m\x1B[30m"

// text mode string literals
#define TXT_M_BOLD          "\x1B[4m"
#define TXT_M_UNDERLN       "\x1B[1m"

//
//  Public functions
//
bool initCmdRenderer(void);

void rebuildScreen(void);

void renderMenu(menu_t* menu, u8 selected);
void renderWorld(u8 worldRenderBuf[WORLD_HEIGHT][WORLD_WIDTH]);

void setFrametime(u64 ftime);

void rModeHideCursor();
void rModeShowCursor();
void rModeSetWTitle(const char* title);

#endif