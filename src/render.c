#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <windows.h>
#include <conio.h>

#include <render.h>
#include <screen.h>
#include <debug/rdebug.h>

const char* g_rPixelColors[16] = {
    "\x1B[40m",     // background black
    "\x1B[41m",     // background red
    "\x1B[42m",     // background green
    "\x1B[43m",     // background yellow
    "\x1B[44m",     // background blue
    "\x1B[45m",     // background magenta
    "\x1B[46m",     // background cyan
    "\x1B[47m",     // background white
    "\x1B[100m",    // background bright black
    "\x1B[101m",    // background bright red
    "\x1B[102m",    // background bright green
    "\x1B[103m",    // background bright yellow
    "\x1B[104m",    // background bright blue
    "\x1B[105m",    // background bright magenta
    "\x1B[106m",    // background bright cyan
    "\x1B[107m"     // background bright white
};

const char* g_rTextColors[16] = {
    "\x1B[30m",     // text black
    "\x1B[31m",     // text red
    "\x1B[32m",     // text green
    "\x1B[33m",     // text yellow
    "\x1B[34m",     // text blue
    "\x1B[35m",     // text magenta
    "\x1B[36m",     // text cyan
    "\x1B[37m",     // text white
    "\x1B[90m",     // text bright black
    "\x1B[91m",     // text bright red
    "\x1B[92m",     // text bright green
    "\x1B[93m",     // text bright yellow
    "\x1B[94m",     // text bright blue
    "\x1B[95m",     // text bright magenta
    "\x1B[96m",     // text bright cyan
    "\x1B[97m"      // text bright white
};

const char* g_rMenuTitle =
    " " PXL_BLACK TXT_BR_WHITE " >>> " TXT_RESET " "
    PXL_BLACK TXT_BR_WHITE " Welcome to Linus' Cmd-Line "
    TXT_RESET " " PXL_RED TXT_BR_WHITE " FLAPPY "
    TXT_RESET " " PXL_YELLOW TXT_BLACK " BIRD "
    TXT_RESET " " PXL_BLACK TXT_BR_WHITE " <<< " TXT_RESET;

const u8 g_bird[12][18] = {
    { 7,  7,  7,  7,  7,  7,  7,  0,  0,  0,  0,  0,  0,  7,  7,  7,  7,  7},
    { 7,  7,  7,  7,  7,  0,  0,  3,  3,  3,  0, 15, 15,  0,  7,  7,  7,  7},
    { 7,  7,  7,  7,  0,  3,  3,  3,  3,  0, 15, 15, 15, 15,  0,  7,  7,  7},
    { 7,  7,  0,  0,  0,  0,  3,  3,  3,  0, 15, 15, 15,  0, 15,  0,  7,  7},
    { 7,  0, 15, 15, 15, 15,  0,  3,  3,  0, 15, 15, 15,  0, 15,  0,  7,  7},
    { 7,  0, 11, 15, 15, 15, 11,  0,  3,  3,  0, 15, 15, 15, 15,  0,  7,  7},
    { 7,  0,  3, 11, 11, 11,  3,  0,  3,  3,  3,  0,  0,  0,  0,  0,  0,  7},
    { 7,  7,  0,  3,  3,  3,  0,  3,  3,  3,  0,  1,  1,  1,  1,  1,  1,  0},
    { 7,  7,  7,  0,  0,  0,  9,  9,  9,  0,  1,  0,  0,  0,  0,  0,  0,  7},
    { 7,  7,  7,  0,  9,  9,  9,  9,  9,  9,  0,  1,  1,  1,  1,  1,  0,  7},
    { 7,  7,  7,  7,  0,  0,  9,  9,  9,  9,  9,  0,  0,  0,  0,  0,  7,  7},
    { 7,  7,  7,  7,  7,  7,  0,  0,  0,  0,  0,  7,  7,  7,  7,  7,  7,  7}
};

u64 g_frametimeMS = 0;

HANDLE g_conOut = NULL;

void setErrorBuf(char* s) {}
void displayError() {}

// setup terminal for virtual processing
bool initCmdRenderer(void)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

    if (hOut == INVALID_HANDLE_VALUE || hIn == INVALID_HANDLE_VALUE)
        return 0;

    DWORD mOrigOut = 0;
    DWORD mOrigIn = 0;

    if (!GetConsoleMode(hOut, &mOrigOut) || !GetConsoleMode(hIn, &mOrigIn))
        return 0;

    DWORD mReqOut = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    DWORD mActOut = mOrigOut | mReqOut;

    if (!SetConsoleMode(hOut, mActOut))
        return 0;

    DWORD mReqIn = ENABLE_VIRTUAL_TERMINAL_INPUT;
    DWORD mActIn = mOrigIn | mReqIn;

    if (!SetConsoleMode(hIn, mActIn))
        return 0;

    g_conOut = hOut;

    rModeHideCursor();
    rModeSetWTitle("C-FLAPPY");

    system("@echo off");
    system("cls");

    char cmdbuf[32];
    snprintf(cmdbuf, 32, "mode con: cols=%d lines=%d", SCREEN_WIDTH, SCREEN_HEIGHT);

    system(cmdbuf);

    return 1;
}

// clear screen and render screen buffer
void rebuildScreen(void)
{
    /*
    if (getScreenState() != 0) {
        setErrorBuf("ERROR: Screen buffer not ready");
        displayError();

        return;
    }
    */

    system("cls");

    fputs(getScreenBuf(), stdout);
}

// render menu with selected item
void renderMenu(menu_t* menu, u8 selected)
{
    screenResetBuf();

    screenBuildHeaderFPS(0);
    screenBuildEmptyLine();
    screenBuildTextLine(g_rMenuTitle, 120);
    
    screenBuildPadding(3);

    for (u8 i = 0; i < 12; i++) {
        screenBuildPixelLine(g_bird[i], 18);
    }

    screenBuildPadding(3);

    char linebuf[128];
    snprintf(linebuf, 128, "   " PXL_BLACK TXT_BR_WHITE " %s: " TXT_RESET, rNullStringWrap(menu->title));

    screenBuildTextLine(linebuf, 24);
    screenBuildEmptyLine();

    const char* tmp;
    u8 ctrlchars;

    for (u8 i = 0; i < menu->len; i++) {
        ctrlchars = 14;

        if (i == selected) {
            tmp = PXL_BR_WHITE "> ";
            ctrlchars += 6;
        } else {
            tmp = " ";
        }

        snprintf(linebuf, 128, "    %s%s" TXT_RESET, tmp, rNullStringWrap(menu->items[i]));

        screenBuildTextLine(linebuf, ctrlchars);
    }

    screenBuildFooter();

    rebuildScreen();
}

void renderWorld(u8 worldRenderBuf[WORLD_HEIGHT][WORLD_WIDTH])
{
    screenResetBuf();

    screenBuildHeaderFPS(g_frametimeMS);

    for (u16 i = 0; i < WORLD_HEIGHT; i++) {
        screenBuildPixelLine(worldRenderBuf[i], WORLD_WIDTH);
    }

    screenBuildFooter();
    rebuildScreen();
}

void setFrametime(u64 ftime)
{
    g_frametimeMS = ftime;
}

void rModeHideCursor()
{
    fputs("\x1B[?25l", stdout);
}

void rModeShowCursor()
{
    fputs("\x1B[?25h", stdout);
}

void rModeSetWTitle(const char* title)
{
    fprintf(stdout, "\x1B]0;%s\x1B\x5C", title);
}